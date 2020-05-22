#define _C_HTTP3
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "http3.h"
#include "log/log.h"

extern void flush_egress(struct http_stream *hs);

void timeout_cb(const int sock, short int which, void *arg)
{
    struct http_stream *hs = arg;
    struct http3_params *http3_params = hs->http3_params;
    
    quiche_conn_on_timeout(http3_params->conn);

    log_debug("timeout");

    flush_egress(hs);

    if (quiche_conn_is_closed(http3_params->conn))
    {
        quiche_stats stats;

        quiche_conn_stats(http3_params->conn, &stats);
        log_debug("connection closed, recv=%zu sent=%zu lost=%zu rtt=%" PRIu64 "ns cwnd=%zu",
                  stats.recv, stats.sent, stats.lost, stats.rtt, stats.cwnd);

        delete_connection(hs);
        http3_connection_cleanup(hs);

        return;
    }
}
