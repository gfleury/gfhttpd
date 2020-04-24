#define _C_HTTP3

#include <fcntl.h>

#include "http3.h"

#include <quiche.h>

int main(int argc, char *argv[])
{
    const char *host = argv[1];
    const char *port = argv[2];

    const struct addrinfo hints = {
        .ai_family = PF_UNSPEC,
        .ai_socktype = SOCK_DGRAM,
        .ai_protocol = IPPROTO_UDP,
    };

    struct addrinfo *local;
    if (getaddrinfo(host, port, &hints, &local) != 0)
    {
        perror("failed to resolve host");
        return -1;
    }

    int sock = http3_init_sock(local);
    if (sock < 0)
    {
        perror("failed to run init_sock()");
        return -1;
    }

    if (http3_init_config() < 0)
    {
        perror("failed to run init_http3_config()");
        return -1;
    }

    struct app_context app_ctx;
    struct connections c;
    c.sock = sock;
    c.h = NULL;
    app_ctx.conns = &c;

    struct event_base *loop = event_base_new();
    assert(loop != NULL);

    int ret = evutil_make_socket_nonblocking(sock);
    assert(ret >= 0);

    app_ctx.evbase = loop;

    struct event *accept_event = event_new(loop, sock, EV_READ | EV_PERSIST, http3_event_cb, &app_ctx);
    assert(accept_event != NULL);

    if (event_add(accept_event, NULL) < 0)
    {
        fprintf(stderr, "Could not create/add a accept event!\n");
        return 1;
    }

    ret = event_base_loop(loop, EVLOOP_NO_EXIT_ON_EMPTY);
    assert(ret == 0);

    event_free(accept_event);
    event_base_free(loop);

    freeaddrinfo(local);

    quiche_h3_config_free(http3_config);

    quiche_config_free(config);

    return 0;
}
