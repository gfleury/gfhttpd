#include "http2.h"
#include "http2_session.h"
#include "ssl.h"

#include <string.h>

#include <netinet/tcp.h>

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

static struct http_stream *create_http2_conn(app_context *app_ctx,
                                             int fd,
                                             struct sockaddr *addr,
                                             int addrlen)
{
    mem_pool mp = mp_new(16 * 1024);
    struct http_stream *hs = mp_calloc(mp, 1, sizeof(*hs));
    hs->mp = mp;

    int val = 1;

    hs->ssl = create_ssl(app_ctx->ssl_ctx);
    // hs->app_ctx = app_ctx;

    http2_session_data *session_data = mp_calloc(mp, 1, sizeof(http2_session_data));

    hs->http2_params = session_data;

    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(val));

    session_data->bev = bufferevent_openssl_socket_new(
        app_ctx->evbase, fd, hs->ssl, BUFFEREVENT_SSL_ACCEPTING,
        BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    bufferevent_enable(session_data->bev, EV_READ | EV_WRITE);

    return hs;
}

/* callback for evconnlistener */
void http2_acceptcb(struct evconnlistener *listener, int fd,
                    struct sockaddr *addr, int addrlen, void *arg)
{
    app_context *app_ctx = (app_context *)arg;
    (void)listener;

    struct http_stream *hs = create_http2_conn(app_ctx, fd, addr, addrlen);

    http2_session_data *session_data = (http2_session_data *)hs->http2_params;

    bufferevent_setcb(session_data->bev, http2_readcb, http2_writecb, http2_eventcb, hs);
}
