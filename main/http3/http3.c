#define _C_HTTP3
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#include <netdb.h>

#include <sys/types.h>

#include "http3.h"
#include "log/log.h"

#define MAX_TOKEN_LEN                     \
    sizeof("quiche") - 1 +                \
        sizeof(struct sockaddr_storage) + \
        QUICHE_MAX_CONN_ID_LEN

quiche_config *pquiche_config = NULL;

quiche_h3_config *http3_config = NULL;

void write_cb(struct http_stream *hs);

void debug_log(const char *line, void *argp)
{
    log_debug("%s", line);
}

int http3_init_sock(struct config *config)
{
    struct sockaddr_in6 sin6;

#ifdef SIN6_LEN
    sin6.sin6_len = sizeof(sin6);
#endif
    sin6.sin6_family = AF_INET6;
    sin6.sin6_flowinfo = 0;
    sin6.sin6_port = htons(atoi(config->listen_port));
    sin6.sin6_addr = in6addr_any;

    quiche_enable_debug_logging(debug_log, NULL);

    int sock = socket(sin6.sin6_family, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("failed to create socket");
        return -1;
    }

    if (fcntl(sock, F_SETFL, O_NONBLOCK) != 0)
    {
        perror("failed to make socket non-blocking");
        return -1;
    }

    if (bind(sock, (struct sockaddr *)&sin6, sizeof(sin6)) < 0)
    {
        perror("failed to connect socket");
        return -1;
    }
    return sock;
}

int http3_init_config(struct config *config)
{

    pquiche_config = quiche_config_new(QUICHE_PROTOCOL_VERSION);
    if (pquiche_config == NULL)
    {
        log_error("failed to create config");
        return -1;
    }

    if (quiche_config_load_cert_chain_from_pem_file(pquiche_config, config->cert_file) != 0)
    {
        log_error("Unable to open/load cert file: %s, errno: %d", config->cert_file, errno);
        return -1;
    }

    if (quiche_config_load_priv_key_from_pem_file(pquiche_config, config->key_file) != 0)
    {
        log_error("Unable to open/load key file: %s, errno: %d", config->key_file, errno);
        return -1;
    }

    quiche_config_set_application_protos(pquiche_config,
                                         (uint8_t *)QUICHE_H3_APPLICATION_PROTOCOL,
                                         sizeof(QUICHE_H3_APPLICATION_PROTOCOL) - 1);

    quiche_config_set_max_idle_timeout(pquiche_config, 5000);
    // quiche_config_set_max_packet_size(config, MAX_DATAGRAM_SIZE);
    quiche_config_set_initial_max_data(pquiche_config, 10485760);
    // quiche_config_set_initial_max_stream_data_bidi_local(config, 1000000);
    quiche_config_set_initial_max_stream_data_bidi_remote(pquiche_config, 1048576);
    quiche_config_set_initial_max_stream_data_uni(pquiche_config, 1048576);
    quiche_config_set_initial_max_streams_bidi(pquiche_config, 128);
    quiche_config_set_initial_max_streams_uni(pquiche_config, 3);
    // quiche_config_set_disable_active_migration(config, true);
    quiche_config_set_cc_algorithm(pquiche_config, QUICHE_CC_RENO);

    http3_config = quiche_h3_config_new();
    if (http3_config == NULL)
    {
        log_error("failed to create HTTP/3 config");
        return -1;
    }
    quiche_h3_config_set_max_header_list_size(http3_config, 16384);
    return 0;
}

struct future_write
{
    struct event *fw_event;
    struct http_stream *hs;
    int64_t stream_id;
    int fd;
    int pos;
    int len;
    bool eof;
};

quiche_h3_header *get_http3_headers(headers *phdrs, mem_pool mp, unsigned int *length, char *http_status, size_t len, char *buf)
{
    *length = length_header(&phdrs);
    headers *current, *tmp;
    int idx = 0;
    quiche_h3_header *h3_hdrs = mp_calloc(mp, *length, sizeof(quiche_h3_header));

    HASH_ITER(hh, phdrs, current, tmp)
    {
        if (strncasecmp(current->name, ":status", current->n_name) == 0)
        {
            current->value = http_status;
            current->n_value = strlen(http_status);

            log_debug("HTTP :status is %.*s", (int)current->n_value, current->value);
        }
        else if (strncasecmp(current->name, "content-length", current->n_name) == 0)
        {
            current->value = buf;
            current->n_value = sprintf(buf, "%zu", len);

            log_debug("Content-Legth is %.*s", (int)current->n_value, current->value);
        }

        h3_hdrs[idx].name = (const uint8_t *)current->name;
        h3_hdrs[idx].name_len = current->n_name;
        h3_hdrs[idx].value = (const uint8_t *)current->value;
        h3_hdrs[idx].value_len = current->n_value;

        idx++;
    }

    return h3_hdrs;
}

void future_write_cb(const int sock, short int which, void *arg)
{
    struct future_write *fw = arg;
    struct http_stream *hs = fw->hs;
    struct http3_params *http3_params = hs->http3_params;
    char buf[14515];

    if (quiche_conn_is_closed(http3_params->conn))
    {
        log_debug("Connection closed, giving up.");
        return;
    }

    if (hs->response.headers_sent == false && hs->response.content_lenght != -1)
    {
        unsigned int n_headers;
        quiche_h3_header *headers = get_http3_headers(hs->response.headers, hs->mp,
                                                      &n_headers, hs->response.http_status, hs->response.content_lenght, buf);

        quiche_h3_send_response(http3_params->http3, http3_params->conn,
                                fw->stream_id, headers, n_headers, false);

        hs->response.headers_sent = true;
        fw->len = hs->response.content_lenght;
    }
    else if (hs->response.headers_sent == false && hs->response.content_lenght == -1)
    {
        struct timeval half_sec = {0, 2000};
        event_add(fw->fw_event, &half_sec);
        return;
    }

    int fd = fw->fd;
    int left = fw->len - fw->pos;

    lseek(fd, fw->pos, SEEK_SET);

    while (fw->pos < fw->len)
    {
        int n = read(fd, buf, sizeof(buf));
        int written = 0;

        if ((fw->pos + n) >= fw->len)
        {
            fw->eof = true;
        }

        written += quiche_h3_send_body(http3_params->http3, http3_params->conn,
                                       fw->stream_id, (uint8_t *)buf, n - written, fw->eof);
        if (written == QUICHE_H3_ERR_DONE)
        {
            log_error("QUICHE FUCKUP!");
            exit(0);
        }

        fw->pos += written;
        left = left - n;
        log_debug("%s %s Written %d read %d, left %d - EOF=%s.", hs->request.method, hs->request.url, written, n, left, fw->eof ? "true" : "false");

        if (written != n)
        {
            log_debug("*** We need to push more but we failed.");

            struct timeval half_sec = {0, 2000};
            event_add(fw->fw_event, &half_sec);
            break;
        }
    }
    flush_egress(hs);

    if (fw->eof)
    {
        log_debug("Cleaning fw->fw_event");
        evtimer_del(fw->fw_event);
        close(fd);
    }

    return;
}

static void set_timeout(struct http_stream *hs, int64_t q_tout)
{
    if (q_tout && hs->timeout_ev)
    {
        time_t tout = (q_tout + 999999) / 1000000;

        hs->timer.tv_sec = (tout / 1000);
        hs->timer.tv_usec = (unsigned int)(tout % 1000) * 1000;

        // Set the boundary to next second just in case
        if (hs->timer.tv_usec >= 1000000)
        {
            hs->timer.tv_sec++;
            hs->timer.tv_usec -= 1000000;
        }

        evtimer_add(hs->timeout_ev, &hs->timer);
    }
}

int send_response(struct http_stream *hs, int64_t stream_id, int fd)
{
    //struct http3_params *http3_params = hs->http3_params;

    struct future_write *fw = mp_calloc(hs->mp, 1, sizeof(struct future_write));
    fw->fd = fd;
    fw->hs = hs;
    fw->pos = 0;
    fw->stream_id = stream_id;
    fw->len = -1;
    fw->eof = false;

    fw->fw_event = mp_calloc(hs->mp, 1, event_get_struct_event_size());
    assert(fw->fw_event);

    evtimer_assign(fw->fw_event, hs->evbase, future_write_cb, fw);
    struct timeval half_sec = {0, 1000};
    event_add(fw->fw_event, &half_sec);
    event_active(fw->fw_event, 0, 0);

    log_debug("content scheduled to be shipped.");
    return 0;
}

void flush_egress(struct http_stream *hs)
{
    static uint8_t out[MAX_DATAGRAM_SIZE];
    struct http3_params *http3_params = hs->http3_params;

    while (1)
    {
        ssize_t written = quiche_conn_send(http3_params->conn, out, sizeof(out));

        if (written == QUICHE_ERR_DONE)
        {
            log_debug("done writing");
            break;
        }

        if (written < 0)
        {
            log_error("failed to create packet: %zd", written);
            return;
        }

        ssize_t sent = sendto(hs->sock, out, written, 0,
                              (struct sockaddr *)&hs->peer_addr,
                              hs->peer_addr_len);

        if (sent == -1 && errno == EISCONN)
        {
            sent = send(hs->sock, out, written, 0);
            errno = 0;
        }

        if (sent != written)
        {
            log_error("failed to send: %d", errno);
            return;
        }

        log_debug("sent %zd bytes", sent);
    }

    int64_t q_tout = quiche_conn_timeout_as_nanos(http3_params->conn);
    set_timeout(hs, q_tout);
}

void http3_cleanup(struct app_context *app_ctx)
{
    struct http_stream *hs, *tmp;
    int c = 0;

    HASH_ITER(hh, connections_iter(), hs, tmp)
    {
        log_debug("Going thru loop connection count %d", c++);
        delete_connection(hs);
        http3_connection_cleanup(hs);
    }

    close(app_ctx->conns->sock);
    free(app_ctx->conns);

    if (app_ctx->evaccept_http3)
    {
        event_del(app_ctx->evaccept_http3);
        event_free(app_ctx->evaccept_http3);
    }

    quiche_h3_config_free(http3_config);
    quiche_config_free(pquiche_config);
}

void http3_connection_cleanup(struct http_stream *hs)
{
    struct http3_params *http3_params = hs->http3_params;

    evtimer_del(hs->timeout_ev);
    event_free(hs->timeout_ev);

    if (http3_params->http3)
    {
        quiche_h3_conn_free(http3_params->http3);
        http3_params->http3 = NULL;
    }

    quiche_conn_free(http3_params->conn);
    http3_params->conn = NULL;

    free(hs->request.authority);
    free(hs->request.method);
    free(hs->request.url);
    free(hs->request.scheme);

    delete_header_all(&hs->request.headers);
    // delete_header_all(hs->response.headers);

    log_debug("Freed the hell out of it!");
    mp_delete(&hs->mp, true);
}