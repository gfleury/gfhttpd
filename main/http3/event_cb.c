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

extern void flush_egress(struct http_stream *conn_io);

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

static struct http_stream *create_conn(struct app_context *app_ctx, uint8_t *odcid, size_t odcid_len)
{
    struct http3_params *http3_params = calloc(1, sizeof(struct http3_params));
    struct http_stream *conn_io = calloc(1, sizeof(*conn_io));

    struct connections *conns = app_ctx->conns;

    if (conn_io == NULL)
    {
        log_error("failed to allocate connection IO");
        return NULL;
    }

    int rng = open("/dev/urandom", O_RDONLY);
    if (rng < 0)
    {
        log_error("failed to open /dev/urandom: %d", errno);
        return NULL;
    }

    ssize_t rand_len = read(rng, conn_io->cid, LOCAL_CONN_ID_LEN);
    close(rng);
    if (rand_len < 0)
    {
        log_error("failed to create connection ID: %d", errno);
        return NULL;
    }

    quiche_conn *conn = quiche_accept(conn_io->cid, LOCAL_CONN_ID_LEN,
                                      odcid, odcid_len, pquiche_config);
    if (conn == NULL)
    {
        log_error("failed to create connection: %d", errno);
        return NULL;
    }

    conn_io->sock = conns->sock;
    conn_io->app_ctx = app_ctx;

    http3_params->conn = conn;

    conn_io->http3_params = http3_params;

    conn_io->timeout_ev = evtimer_new(app_ctx->evbase, timeout_cb, conn_io);
    evtimer_add(conn_io->timeout_ev, &conn_io->timer);

    HASH_ADD(hh, conns->h, cid, LOCAL_CONN_ID_LEN, conn_io);

    log_debug("new connection");

    return conn_io;
}

static int for_each_header(uint8_t *name, size_t name_len, uint8_t *value, size_t value_len, void *argp)
{
    struct http_stream *conn_io = argp;
    assert(conn_io != NULL);

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
                conn_io->request.path = strndup((char *)value, value_len);
                return !check_path(conn_io->request.path);
            }
            break;

        case 6:
            if (memcmp(name, "method", sizeof("method") - 1) == 0)
            {
                conn_io->request.method = strndup((char *)value, value_len);
                return 0;
            }

            if (memcmp(name, "scheme", sizeof("scheme") - 1) == 0)
            {
                conn_io->request.scheme = strndup((char *)value, value_len);
                return 0;
            }
            break;

        case 9:
            if (memcmp(name, "authority", sizeof("authority") - 1) == 0)
            {
                conn_io->request.authority = strndup((char *)value, value_len);
                return 0;
            }

            break;
        }
    }
    return 0;
}

void http3_event_cb(const int sock, short int which, void *arg)
{
    struct http_stream *tmp, *conn_io = NULL;

    struct app_context *app_ctx = arg;
    struct connections *conns = app_ctx->conns;

    static uint8_t buf[65535];
    static uint8_t out[MAX_DATAGRAM_SIZE];

    while (1)
    {
        struct sockaddr_storage peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, peer_addr_len);

        ssize_t read = recvfrom(conns->sock, buf, sizeof(buf), 0,
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

        HASH_FIND(hh, conns->h, dcid, dcid_len, conn_io);

        if (conn_io == NULL)
        {
            if (!quiche_version_is_supported(version))
            {
                log_debug("version negotiation");

                ssize_t written = quiche_negotiate_version(scid, scid_len,
                                                           dcid, dcid_len,
                                                           out, sizeof(out));

                if (written < 0)
                {
                    log_error("failed to create vneg packet: %zd",
                              written);
                    return;
                }

                ssize_t sent = sendto(conns->sock, out, written, 0,
                                      (struct sockaddr *)&peer_addr,
                                      peer_addr_len);
                if (sent != written)
                {
                    log_error("failed to send: %d", errno);
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

                ssize_t sent = sendto(conns->sock, out, written, 0,
                                      (struct sockaddr *)&peer_addr,
                                      peer_addr_len);
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

            conn_io = create_conn(app_ctx, odcid, odcid_len);
            if (conn_io == NULL)
            {
                return;
            }

            memcpy(&conn_io->peer_addr, &peer_addr, peer_addr_len);
            conn_io->peer_addr_len = peer_addr_len;
        }

        struct http3_params *http3_params = conn_io->http3_params;

        ssize_t done = quiche_conn_recv(http3_params->conn, buf, read);

        if (done == QUICHE_ERR_DONE)
        {
            log_debug("done reading");
            break;
        }

        if (done < 0)
        {
            log_error("failed to process packet: %zd", done);
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
                    int rc = quiche_h3_event_for_each_header(ev, for_each_header, conn_io);

                    if (rc != 0)
                    {
                        log_error("failed to process headers");
                        // TODO Ship error.
                    }

                    quiche_h3_header headers[] = {
                        {
                            .name = (const uint8_t *)":status",
                            .name_len = sizeof(":status") - 1,

                            .value = (const uint8_t *)"200",
                            .value_len = sizeof("200") - 1,
                        },
                        {
                            .name = (const uint8_t *)"server",
                            .name_len = sizeof("server") - 1,

                            .value = (const uint8_t *)"quiche",
                            .value_len = sizeof("quiche") - 1,
                        },
                        {
                            .name = (const uint8_t *)"content-length",
                            .name_len = sizeof("content-length") - 1,

                            .value = (const uint8_t *)"5",
                            .value_len = sizeof("5") - 1,
                        },
                    };

                    int fd = root_router(conn_io->request.path);
                    if (fd == -1)
                    {
                        log_error("%s %s We are fucked, unable to root_router.", conn_io->request.method, conn_io->request.path);
                        // TODO Ship error.
                        // if (error_reply(session, stream_data) != 0)
                        // {
                        //     return NGHTTP2_ERR_CALLBACK_FAILURE;
                        // }
                        return;
                    }

                    send_response(conn_io, stream_id, headers, fd);
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

    HASH_ITER(hh, conns->h, conn_io, tmp)
    {
        struct http3_params *http3_params = conn_io->http3_params;
        flush_egress(conn_io);

        if (quiche_conn_is_closed(http3_params->conn))
        {
            quiche_stats stats;

            quiche_conn_stats(http3_params->conn, &stats);
            log_info("connection closed, recv=%zu sent=%zu lost=%zu rtt=%" PRIu64 "ns cwnd=%zu",
                     stats.recv, stats.sent, stats.lost, stats.rtt, stats.cwnd);

            HASH_DELETE(hh, conns->h, conn_io);

            evtimer_del(conn_io->timeout_ev);

            quiche_conn_free(http3_params->conn);

            log_debug("Freeing the hell out of it.");
            free(conn_io->request.authority);
            free(conn_io->request.method);
            free(conn_io->request.path);
            free(conn_io->request.scheme);
            free(conn_io);
        }
    }
}