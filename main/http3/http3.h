#include <event2/event-config.h>
#include <event2/event.h>

#include <assert.h>

#include "http_stream/http_stream.h"

#ifdef _C_HTTP3
#include <quiche.h>

#define MAX_DATAGRAM_SIZE 1200

#define MAX_TOKEN_LEN                     \
    sizeof("quiche") - 1 +                \
        sizeof(struct sockaddr_storage) + \
        QUICHE_MAX_CONN_ID_LEN

struct http3_params
{
    quiche_conn *conn;
    quiche_h3_conn *http3;
};

extern void debug_log(const char *line, void *argp);
extern quiche_config *pquiche_config;
extern quiche_h3_config *http3_config;
int send_response(struct http_stream *conn_io, int64_t stream_id, int fd);
void flush_egress(struct http_stream *conn_io);
#endif

void http3_event_cb(const int sock, short int which, void *arg);
int http3_init_sock(struct addrinfo *local);
int http3_init_config();
void http3_start_listen(struct event_base *evbase, const char *service,
                        app_context *app_ctx);