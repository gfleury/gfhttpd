#define _C_HTTP3
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "http3.h"

extern void flush_egress(struct http_stream *conn_io);

void timeout_cb(const int sock, short int which, void *arg)
{
    struct http_stream *conn_io = arg;
    struct http3_params *http3_params = conn_io->http3_params;
    struct app_context *app_ctx = conn_io->app_ctx;
    struct connections *conns = app_ctx->conns;

    quiche_conn_on_timeout(http3_params->conn);

    fprintf(stderr, "timeout\n");

    flush_egress(conn_io);

    if (quiche_conn_is_closed(http3_params->conn))
    {
        quiche_stats stats;

        quiche_conn_stats(http3_params->conn, &stats);
        fprintf(stderr, "connection closed, recv=%zu sent=%zu lost=%zu rtt=%" PRIu64 "ns cwnd=%zu\n",
                stats.recv, stats.sent, stats.lost, stats.rtt, stats.cwnd);

        HASH_DELETE(hh, conns->h, conn_io);

        evtimer_del(conn_io->timeout_ev);
        quiche_conn_free(http3_params->conn);
        free(conn_io);

        return;
    }
}
