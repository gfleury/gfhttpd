// Copyright (C) 2019, Cloudflare, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <event2/event.h>

#include <quiche.h>

#define _C_HTTP3
#include "http3/http3.h"

#define LOCAL_CONN_ID_LEN 16

struct conn_io
{
    struct event *timer;

    int sock;

    quiche_conn *conn;

    quiche_h3_conn *http3;

    struct event_base *loop;

    quiche_config *config;

    SSL_CTX *ctx;

    SSL *ssl;
};

static int client_write(struct conn_io *conn_io)
{
    static uint8_t out[MAX_DATAGRAM_SIZE];
    while (1)
    {
        ssize_t written = quiche_conn_send(conn_io->conn, out, sizeof(out));

        if (written == QUICHE_ERR_DONE)
        {
            fprintf(stderr, "Client: done writing\n");
            break;
        }

        if (written < 0)
        {
            fprintf(stderr, "Client: failed to create packet: %zd\n", written);
            return -1;
        }

        ssize_t sent = send(conn_io->sock, out, written, 0);
        if (sent != written)
        {
            perror("failed to send");
            return -1;
        }

        fprintf(stderr, "Client: sent %zd bytes\n", sent);
    }
    return 0;
}

static void client_flush_egress(struct event_base *loop, struct conn_io *conn_io)
{

    if (client_write(conn_io) == -1)
    {
        return;
    }

    double q_tout = quiche_conn_timeout_as_nanos(conn_io->conn);
    // conn_io->timer.repeat = t;
    time_t tout = (q_tout + 999999) / 1000000;

    struct timeval t = {.tv_sec = (tout / 1000), .tv_usec = (unsigned int)(tout % 1000) * 1000};

    event_add(conn_io->timer, &t);
}

static int for_each_header(uint8_t *name, size_t name_len,
                           uint8_t *value, size_t value_len,
                           void *argp)
{
    fprintf(stderr, "Client: got HTTP header: %.*s=%.*s\n",
            (int)name_len, name, (int)value_len, value);

    return 0;
}

static void client_recv_cb(const int sock, short int which, void *arg)
{
    static bool req_sent = false;

    struct conn_io *conn_io = arg;

    static uint8_t buf[65535];

    while (1)
    {
        ssize_t read = recv(conn_io->sock, buf, sizeof(buf), 0);

        if (read < 0)
        {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
            {
                fprintf(stderr, "Client: recv would block\n");
                break;
            }

            perror("Client: failed to read");
            return;
        }

        fprintf(stderr, "Client: read %zd bytes\n", read);

        ssize_t done = quiche_conn_recv(conn_io->conn, buf, read);

        if (done == QUICHE_ERR_DONE)
        {
            fprintf(stderr, "done reading");
            continue;
        }

        if (done < 0)
        {
            fprintf(stderr, "Client: failed to process packet: %zd\n", done);
            continue;
        }

        fprintf(stderr, "Client: recv %zd bytes\n", done);
    }

    fprintf(stderr, "Client: done reading\n");

    client_write(conn_io);

    if (quiche_conn_is_closed(conn_io->conn))
    {
        fprintf(stderr, "Client: connection closed\n");

        quiche_h3_conn_free(conn_io->http3);
        quiche_conn_free(conn_io->conn);
        quiche_config_free(conn_io->config);
        // SSL_free(conn_io->ssl);
        SSL_CTX_free(conn_io->ctx);

        event_del(conn_io->timer);
        event_free(conn_io->timer);

        event_base_loopbreak(conn_io->loop);
        free(conn_io);
        return;
    }

    if (quiche_conn_is_established(conn_io->conn))
    {
        const uint8_t *app_proto;
        size_t app_proto_len;

        quiche_conn_application_proto(conn_io->conn, &app_proto, &app_proto_len);

        fprintf(stderr, "Client: connection established: %.*s\n",
                (int)app_proto_len, app_proto);

        quiche_h3_config *config = quiche_h3_config_new();
        if (config == NULL)
        {
            fprintf(stderr, "Client: failed to create HTTP/3 config\n");
            return;
        }

        if (conn_io->http3 == NULL)
        {
            conn_io->http3 = quiche_h3_conn_new_with_transport(conn_io->conn, config);
            if (conn_io->http3 == NULL)
            {
                fprintf(stderr, "Client: failed to create HTTP/3 connection\n");
                return;
            }
        }

        quiche_h3_config_free(config);

        quiche_h3_header headers[] = {
            {
                .name = (const uint8_t *)":method",
                .name_len = sizeof(":method") - 1,

                .value = (const uint8_t *)"GET",
                .value_len = sizeof("GET") - 1,
            },

            {
                .name = (const uint8_t *)":scheme",
                .name_len = sizeof(":scheme") - 1,

                .value = (const uint8_t *)"https",
                .value_len = sizeof("https") - 1,
            },

            {
                .name = (const uint8_t *)":authority",
                .name_len = sizeof(":authority") - 1,

                .value = (const uint8_t *)"dd",
                .value_len = strlen("dd"),
            },

            {
                .name = (const uint8_t *)":path",
                .name_len = sizeof(":path") - 1,

                .value = (const uint8_t *)"/",
                .value_len = sizeof("/") - 1,
            },

            {
                .name = (const uint8_t *)"user-agent",
                .name_len = sizeof("user-agent") - 1,

                .value = (const uint8_t *)"quiche",
                .value_len = sizeof("quiche") - 1,
            },
        };

        int64_t stream_id = quiche_h3_send_request(conn_io->http3,
                                                   conn_io->conn,
                                                   headers, 5, true);

        fprintf(stderr, "Client: sent HTTP request %" PRId64 "\n", stream_id);

        req_sent = true;
    }

    if (quiche_conn_is_established(conn_io->conn))
    {
        quiche_h3_event *ev;

        while (1)
        {
            int64_t s = quiche_h3_conn_poll(conn_io->http3,
                                            conn_io->conn,
                                            &ev);

            if (s < 0)
            {
                break;
            }

            switch (quiche_h3_event_type(ev))
            {
            case QUICHE_H3_EVENT_HEADERS:
            {
                int rc = quiche_h3_event_for_each_header(ev, for_each_header,
                                                         NULL);

                if (rc != 0)
                {
                    fprintf(stderr, "Client: failed to process headers");
                }

                break;
            }

            case QUICHE_H3_EVENT_DATA:
            {
                ssize_t len = quiche_h3_recv_body(conn_io->http3,
                                                  conn_io->conn, s,
                                                  buf, sizeof(buf));
                if (len <= 0)
                {
                    break;
                }

                printf("%.*s", (int)len, buf);
                break;
            }

            case QUICHE_H3_EVENT_FINISHED:
                if (quiche_conn_close(conn_io->conn, true, 0, NULL, 0) < 0)
                {
                    fprintf(stderr, "Client: failed to close connection\n");
                }
                break;
            }

            quiche_h3_event_free(ev);
        }
    }

    client_flush_egress(conn_io->loop, conn_io);
}

static void client_timeout_cb(const int sock, short int which, void *arg)
{
    struct conn_io *conn_io = arg;
    quiche_conn_on_timeout(conn_io->conn);

    fprintf(stderr, "Client: timeout\n");

    client_flush_egress(conn_io->loop, conn_io);

    if (quiche_conn_is_closed(conn_io->conn))
    {
        quiche_stats stats;

        quiche_conn_stats(conn_io->conn, &stats);

        fprintf(stderr, "Client: connection closed, recv=%zu sent=%zu lost=%zu rtt=%" PRIu64 "ns\n",
                stats.recv, stats.sent, stats.lost, stats.rtt);

        quiche_h3_conn_free(conn_io->http3);
        quiche_conn_free(conn_io->conn);
        quiche_config_free(conn_io->config);
        // SSL_free(conn_io->ssl);
        SSL_CTX_free(conn_io->ctx);

        event_del(conn_io->timer);
        event_free(conn_io->timer);

        while (length_connection() != 0)
        {
            fprintf(stderr, "Sleeping 1 second while wait for conenctions from %d to zero....\n", length_connection());
            sleep(1);
        }
        event_base_loopbreak(conn_io->loop);
        free(conn_io);
        return;
    }
}

struct event *http3_client(struct event_base *loop, int sock)
{
    if (fcntl(sock, F_SETFL, O_NONBLOCK) != 0)
    {
        perror("failed to make socket non-blocking");
        return NULL;
    }

    struct conn_io *conn_io = calloc(1, sizeof(*conn_io));
    if (conn_io == NULL)
    {
        fprintf(stderr, "Client: failed to allocate connection IO\n");
        return NULL;
    }
    conn_io->http3 = NULL;

    conn_io->config = quiche_config_new(QUICHE_PROTOCOL_VERSION);

    if (conn_io->config == NULL)
    {
        fprintf(stderr, "Client: failed to create config\n");
        return NULL;
    }

    quiche_config_set_application_protos(conn_io->config,
                                         (uint8_t *)QUICHE_H3_APPLICATION_PROTOCOL,
                                         sizeof(QUICHE_H3_APPLICATION_PROTOCOL) - 1);

    quiche_config_set_max_idle_timeout(conn_io->config, 10000);
    quiche_config_set_max_packet_size(conn_io->config, MAX_DATAGRAM_SIZE);
    quiche_config_set_initial_max_data(conn_io->config, 10485760);
    quiche_config_set_initial_max_stream_data_bidi_local(conn_io->config, 1048576);
    quiche_config_set_initial_max_stream_data_bidi_remote(conn_io->config, 1048576);
    quiche_config_set_initial_max_stream_data_uni(conn_io->config, 1048576);
    quiche_config_set_initial_max_streams_bidi(conn_io->config, 128);
    quiche_config_set_initial_max_streams_uni(conn_io->config, 3);
    quiche_config_set_disable_active_migration(conn_io->config, true);

    if (getenv("SSLKEYLOGFILE"))
    {
        quiche_config_log_keys(conn_io->config);
    }

    // ABC: old config creation here

    uint8_t scid[LOCAL_CONN_ID_LEN];
    int rng = open("/dev/urandom", O_RDONLY);
    if (rng < 0)
    {
        perror("Client: failed to open /dev/urandom");
        return NULL;
    }

    ssize_t rand_len = read(rng, &scid, sizeof(scid));
    if (rand_len < 0)
    {
        perror("Client: failed to create connection ID");
        return NULL;
    }

    // quiche_conn *conn = quiche_connect("blog.cloudflare.com", (const uint8_t *)scid, sizeof(scid), config);
    conn_io->ctx = SSL_CTX_new(TLS_method());
    SSL_CTX_set_alpn_protos(conn_io->ctx, (uint8_t *)QUICHE_H3_APPLICATION_PROTOCOL,
                            sizeof(QUICHE_H3_APPLICATION_PROTOCOL) - 1);
    conn_io->ssl = SSL_new(conn_io->ctx);
    SSL_set_connect_state(conn_io->ssl);

    SSL_set_tlsext_host_name(conn_io->ssl, "localhost");

    quiche_conn *conn = quiche_conn_new_with_tls((const uint8_t *)scid, sizeof(scid), NULL, 0, conn_io->config, conn_io->ssl, false);

    if (conn == NULL)
    {
        fprintf(stderr, "Client: failed to create connection\n");
        return NULL;
    }

    conn_io->sock = sock;
    conn_io->conn = conn;

    struct event *watcher = event_new(loop, sock, EV_READ | EV_PERSIST, client_recv_cb, conn_io);
    assert(watcher != NULL);

    conn_io->loop = loop;

    if (event_add(watcher, NULL) < 0)
    {
        fprintf(stderr, "Client: Could not create/add a watcher event!\n");
        return NULL;
    }

    conn_io->timer = evtimer_new(loop, client_timeout_cb, conn_io);
    event_add(conn_io->timer, 0);
    event_active(conn_io->timer, 0, 0);

    return watcher;
}