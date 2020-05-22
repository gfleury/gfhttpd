#define _C_HTTP3
#include <fcntl.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "http3.h"

#include "log/log.h"
#include "router/router.h"

extern quiche_config *pquiche_config;

extern quiche_h3_config *http3_config;

extern void timeout_cb(const int sock, short int which, void *arg);
extern void http3_write_cb(const int sock, short int which, void *arg);

extern void flush_egress(struct http_stream *hs);

static void mint_token(const uint8_t *dcid, size_t dcid_len,
                       struct sockaddr_storage *addr, socklen_t addr_len,
                       uint8_t *token, size_t *token_len)
{
    memcpy(token, "quiche", sizeof("quiche") - 1);
    memcpy(token + sizeof("quiche") - 1, addr, addr_len);
    memcpy(token + sizeof("quiche") - 1 + addr_len, dcid, dcid_len);

    *token_len = sizeof("quiche") - 1 + addr_len + dcid_len;
}

static bool validate_token(const uint8_t *token, size_t token_len,
                           struct sockaddr_storage *addr, socklen_t addr_len,
                           uint8_t *odcid, size_t *odcid_len)
{
    if ((token_len < sizeof("quiche") - 1) ||
        memcmp(token, "quiche", sizeof("quiche") - 1))
    {
        return false;
    }

    token += sizeof("quiche") - 1;
    token_len -= sizeof("quiche") - 1;

    if ((token_len < addr_len) || memcmp(token, addr, addr_len))
    {
        return false;
    }

    token += addr_len;
    token_len -= addr_len;

    if (*odcid_len < token_len)
    {
        return false;
    }

    memcpy(odcid, token, token_len);
    *odcid_len = token_len;

    return true;
}

static struct http_stream *create_conn(int sock, uint8_t *odcid, size_t odcid_len)
{
    mem_pool mp = mp_new(16 * 1024);
    assert(mp);

    struct http3_params *http3_params = mp_calloc(mp, 1, sizeof(struct http3_params));
    struct http_stream *hs = mp_calloc(mp, 1, sizeof(*hs));
    assert(hs);
    hs->mp = mp;

    if (hs == NULL)
    {
        log_error("failed to allocate http stream");
        return NULL;
    }

    int rng = open("/dev/urandom", O_RDONLY);
    if (rng < 0)
    {
        log_error("failed to open /dev/urandom: %d", errno);
        return NULL;
    }

    ssize_t rand_len = read(rng, hs->cid, LOCAL_CONN_ID_LEN);
    close(rng);
    if (rand_len < 0)
    {
        log_error("failed to create connection ID: %d", errno);
        return NULL;
    }

    http3_params->conn = quiche_accept(hs->cid, LOCAL_CONN_ID_LEN,
                                       odcid, odcid_len, pquiche_config);
    if (http3_params->conn == NULL)
    {
        log_error("failed to create connection: %d", errno);
        return NULL;
    }

    hs->sock = sock;

    // Init Headers
    hs->request.headers = NULL;
    hs->response.headers = NULL;
    hs->response.http_status = "200";

    hs->http3_params = http3_params;

    log_debug("new connection");

    return hs;
}

static int for_each_header(uint8_t *name, size_t name_len, uint8_t *value, size_t value_len, void *argp)
{
    struct http_stream *hs = argp;
    assert(hs != NULL);

    log_debug("got HTTP header: %.*s=%.*s", (int)name_len, name, (int)value_len, value);

    // Check for pseudo-headers
    if (*name == ':')
    {
        name++;
        name_len--;
        switch (name_len)
        {
        case 4:
            if (memcmp(name, "path", sizeof("path") - 1) == 0)
            {
                hs->request.url = strndup((char *)value, value_len);
                return !check_path(hs->request.url);
            }
            break;

        case 6:
            if (memcmp(name, "method", sizeof("method") - 1) == 0)
            {
                hs->request.method = strndup((char *)value, value_len);
                return 0;
            }

            if (memcmp(name, "scheme", sizeof("scheme") - 1) == 0)
            {
                hs->request.scheme = strndup((char *)value, value_len);
                return 0;
            }
            break;

        case 9:
            if (memcmp(name, "authority", sizeof("authority") - 1) == 0)
            {
                hs->request.authority = strndup((char *)value, value_len);
                return 0;
            }

            break;
        }
    }
    else
    { // Normal headers
        insert_header(&hs->request.headers, hs->mp, strndup((char *)name, name_len), name_len, strndup((char *)value, value_len), value_len);
    }
    return 0;
}

void http3_event_cb(const int sock, short int which, void *arg)
{
    struct http_stream *tmp, *hs = NULL;

    struct app_context *app_ctx = arg;
    struct connections *conns = app_ctx->conns;

    static uint8_t buf[65535];
    static uint8_t out[MAX_DATAGRAM_SIZE];

    while (1)
    {
        struct sockaddr_storage peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, peer_addr_len);

        ssize_t read = recvfrom(sock, buf, sizeof(buf), 0,
                                (struct sockaddr *)&peer_addr,
                                &peer_addr_len);

        if (read < 0)
        {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
            {
                log_debug("recv would block");
                break;
            }

            log_error("failed to read");
            return;
        }

        log_debug("Read %zu bytes", read);

        uint8_t type;
        uint32_t version;

        uint8_t scid[QUICHE_MAX_CONN_ID_LEN];
        size_t scid_len = sizeof(scid);

        uint8_t dcid[QUICHE_MAX_CONN_ID_LEN];
        size_t dcid_len = sizeof(dcid);

        uint8_t odcid[QUICHE_MAX_CONN_ID_LEN];
        size_t odcid_len = sizeof(odcid);

        uint8_t token[MAX_TOKEN_LEN];
        size_t token_len = sizeof(token);

        int rc = quiche_header_info(buf, read, LOCAL_CONN_ID_LEN, &version,
                                    &type, scid, &scid_len, dcid, &dcid_len,
                                    token, &token_len);
        if (rc < 0)
        {
            log_error("failed to parse header: %d", rc);
            return;
        }

        HASH_FIND(hh, conns->http_streams, dcid, dcid_len, hs);

        if (hs == NULL)
        {
            if (!quiche_version_is_supported(version))
            {
                log_debug("version negotiation, %zu is not supported", version);

                ssize_t written = quiche_negotiate_version(scid, scid_len,
                                                           dcid, dcid_len,
                                                           out, sizeof(out));

                if (written < 0)
                {
                    log_error("failed to create vneg packet: %zd",
                              written);
                    return;
                }

                ssize_t sent = sendto(sock, out, written, 0,
                                      (struct sockaddr *)&peer_addr,
                                      peer_addr_len);

                if (sent == -1 && errno == EISCONN)
                {
                    sent = send(sock, out, written, 0);
                    errno = 0;
                }

                if (sent != written)
                {
                    log_error("failed to send: errno=%d sent=%d written=%d", errno, sent, written);
                    return;
                }

                log_debug("sent %zd bytes", sent);
                return;
            }

            if (token_len == 0)
            {
                log_debug("stateless retry");

                mint_token(dcid, dcid_len, &peer_addr, peer_addr_len,
                           token, &token_len);

                ssize_t written = quiche_retry(scid, scid_len,
                                               dcid, dcid_len,
                                               dcid, dcid_len,
                                               token, token_len,
                                               out, sizeof(out));

                if (written < 0)
                {
                    log_error("failed to create retry packet: %zd",
                              written);
                    return;
                }

                ssize_t sent = sendto(sock, out, written, 0,
                                      (struct sockaddr *)&peer_addr,
                                      peer_addr_len);

                if (sent == -1 && errno == EISCONN)
                {
                    sent = send(sock, out, written, 0);
                    errno = 0;
                }

                if (sent != written)
                {
                    log_error("failed to send: %d", errno);
                    return;
                }

                log_debug("sent %zd bytes", sent);
                return;
            }

            if (!validate_token(token, token_len, &peer_addr, peer_addr_len,
                                odcid, &odcid_len))
            {
                log_error("invalid address validation token");
                return;
            }

            hs = create_conn(sock, odcid, odcid_len);
            if (hs == NULL)
            {
                return;
            }
            // TODO: move all ev timers to hs structure and remove evbase dependency
            hs->evbase = app_ctx->evbase;
            hs->timeout_ev = evtimer_new(app_ctx->evbase, timeout_cb, hs);
            assert(hs->timeout_ev);

            evtimer_add(hs->timeout_ev, &hs->timer);

            mem_pool mp = hs->mp;
            HASH_ADD(hh, conns->http_streams, cid, LOCAL_CONN_ID_LEN, hs);
            hs->head = conns->http_streams;

            memcpy(&hs->peer_addr, &peer_addr, peer_addr_len);
            hs->peer_addr_len = peer_addr_len;
        }

        struct http3_params *http3_params = hs->http3_params;

        ssize_t done = quiche_conn_recv(http3_params->conn, buf, read);

        if (done == QUICHE_ERR_DONE)
        {
            log_debug("done reading");
            break;
        }

        if (done < 0)
        {
            log_error("failed to process packet: %zd", done);
            HASH_DELETE(hh, conns->http_streams, hs);
            http3_connection_cleanup(hs);
            return;
        }

        log_debug("recv %zd bytes", done);

        if (quiche_conn_is_established(http3_params->conn))
        {
            quiche_h3_event *ev;

            if (http3_params->http3 == NULL)
            {
                http3_params->http3 = quiche_h3_conn_new_with_transport(http3_params->conn, http3_config);

                if (http3_params->http3 == NULL)
                {
                    log_error("failed to create HTTP/3 connection");
                    return;
                }
            }

            while (1)
            {
                int64_t stream_id = quiche_h3_conn_poll(http3_params->http3, http3_params->conn, &ev);

                if (stream_id < 0)
                {
                    break;
                }

                switch (quiche_h3_event_type(ev))
                {
                case QUICHE_H3_EVENT_HEADERS:
                {
                    int rc = quiche_h3_event_for_each_header(ev, for_each_header, hs);

                    if (rc != 0)
                    {
                        log_error("failed to process headers");
                        // TODO Ship error.
                    }

                    // Init Response
                    insert_header(&hs->response.headers, hs->mp, server_status.name, server_status.n_name, server_status.value, server_status.n_value);
                    insert_header(&hs->response.headers, hs->mp, server_header.name, server_header.n_name, server_header.value, server_header.n_value);
                    insert_header(&hs->response.headers, hs->mp, "content-length", sizeof("content-length") - 1, "5", sizeof("5") - 1);

                    hs->response.content_lenght = -1;

                    int fd = root_router(hs->evbase, hs);
                    if (fd == -1)
                    {
                        log_error("%s %s We are fucked, unable to root_router.", hs->request.method, hs->request.url);
                        return;
                    }

                    send_response(hs, stream_id, fd);
                    break;
                }

                case QUICHE_H3_EVENT_DATA:
                {
                    log_debug("got HTTP data");
                    break;
                }

                case QUICHE_H3_EVENT_FINISHED:

                    break;
                }

                quiche_h3_event_free(ev);
            }
        }
    }

    int c = 0;
    HASH_ITER(hh, conns->http_streams, hs, tmp)
    {
        log_debug("Going thru loop connection count %d", c++);
        struct http3_params *http3_params = hs->http3_params;
        flush_egress(hs);

        if (quiche_conn_is_closed(http3_params->conn))
        {
            quiche_stats stats;

            quiche_conn_stats(http3_params->conn, &stats);
            log_info("connection closed, recv=%zu sent=%zu lost=%zu rtt=%" PRIu64 "ns cwnd=%zu",
                     stats.recv, stats.sent, stats.lost, stats.rtt, stats.cwnd);

            HASH_DELETE(hh, conns->http_streams, hs);
            http3_connection_cleanup(hs);
        }
    }
}