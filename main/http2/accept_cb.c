#include "http2.h"
#include "http2_session.h"
#include "ssl.h"

#include <string.h>

#include <netinet/tcp.h>

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

static http2_session_data *create_http2_session_data(app_context *app_ctx,
                                                     int fd,
                                                     struct sockaddr *addr,
                                                     int addrlen);

/* callback for evconnlistener */
void http2_acceptcb(struct evconnlistener *listener, int fd,
                    struct sockaddr *addr, int addrlen, void *arg)
{
    app_context *app_ctx = (app_context *)arg;
    http2_session_data *session_data;
    (void)listener;

    session_data = create_http2_session_data(app_ctx, fd, addr, addrlen);

    bufferevent_setcb(session_data->bev, http2_readcb, http2_writecb, http2_eventcb, session_data);
}

static http2_session_data *create_http2_session_data(app_context *app_ctx,
                                                     int fd,
                                                     struct sockaddr *addr,
                                                     int addrlen)
{
    int rv;
    http2_session_data *session_data;
    SSL *ssl;
    char host[NI_MAXHOST];
    int val = 1;

    ssl = create_ssl(app_ctx->ssl_ctx);
    session_data = malloc(sizeof(http2_session_data));
    memset(session_data, 0, sizeof(http2_session_data));
    session_data->app_ctx = app_ctx;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(val));
    session_data->bev = bufferevent_openssl_socket_new(
        app_ctx->evbase, fd, ssl, BUFFEREVENT_SSL_ACCEPTING,
        BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    bufferevent_enable(session_data->bev, EV_READ | EV_WRITE);
    rv = getnameinfo(addr, (socklen_t)addrlen, host, sizeof(host), NULL, 0,
                     NI_NUMERICHOST);
    if (rv != 0)
    {
        session_data->client_addr = strdup("(unknown)");
    }
    else
    {
        session_data->client_addr = strdup(host);
    }

    return session_data;
}