#include <err.h>

#include <event2/bufferevent.h>
#include <event2/listener.h>

#include <nghttp2/nghttp2.h>

#include "http_stream/http_stream.h"

#define ARRLEN(x) (sizeof(x) / sizeof(x[0]))

#define MAKE_NV(NAME, VALUE)                                                    \
    {                                                                           \
        (uint8_t *)NAME, (uint8_t *)VALUE, sizeof(NAME) - 1, sizeof(VALUE) - 1, \
            NGHTTP2_NV_FLAG_NONE                                                \
    }

void http2_acceptcb(struct evconnlistener *listener, int fd,
                    struct sockaddr *addr, int addrlen, void *arg);

void http2_readcb(struct bufferevent *bev, void *ptr);

void http2_writecb(struct bufferevent *bev, void *ptr);

void http2_eventcb(struct bufferevent *bev, short events, void *ptr);

int http2_start_listen(struct event_base *evbase, const char *service,
                       app_context *app_ctx);
