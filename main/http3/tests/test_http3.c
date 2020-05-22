#include <fcntl.h>
#include <stdio.h>  /* printf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strcpy */
#include <unistd.h>

#include <assert.h>

#include <pthread.h>

#define _C_HTTP3
#include "http3/http3.h"

#define handle_error_en(en, msg) \
    do                           \
    {                            \
        errno = en;              \
        perror(msg);             \
        exit(EXIT_FAILURE);      \
    } while (0)

struct event *http3_client(struct event_base *loop, int sock);

int test_event_cb()
{
    struct app_context app_ctx;

    struct config config = {
        .routes = NULL,
        .mp = NULL,
        .cert_file = "cert/example-com.cert.pem",
        .key_file = "cert/example-com.key.pem",
    };

    app_ctx.config = &config;

    if (http3_init_config(app_ctx.config) < 0)
    {
        printf("failed to run init_http3_config(), errno: %d", errno);
        return 1;
    }

    int fds[2];
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fds) != 0)
    {
        perror("cannot create socket pair");
        return -1;
    }
    if (fcntl(fds[0], F_SETFL, O_NONBLOCK) != 0)
    {
        perror("failed to make socket non-blocking");
        return -1;
    }
    if (fcntl(fds[1], F_SETFL, O_NONBLOCK) != 0)
    {
        perror("failed to make socket non-blocking");
        return -1;
    }

    app_ctx.evbase = event_base_new();

    // HTTP3 client event loop
    struct event *http_client = http3_client(app_ctx.evbase, fds[1]);
    assert(http_client != NULL);

    app_ctx.conns = calloc(1, sizeof(struct connections));
    app_ctx.conns->sock = fds[0];

    struct event *watcher = event_new(app_ctx.evbase, fds[0], EV_READ | EV_PERSIST, http3_event_cb, &app_ctx);
    assert(watcher != NULL);

    if (event_add(watcher, NULL) < 0)
    {
        fprintf(stderr, "Client: Could not create/add a watcher event!\n");
        return 1;
    }

    assert(event_base_loop(app_ctx.evbase, 0) == 0);

    close(fds[0]);
    close(fds[1]);

    event_free(http_client);
    event_free(watcher);

    event_base_free(app_ctx.evbase);

    http3_cleanup(&app_ctx);

    return (EXIT_SUCCESS);
}

int main()
{
    return (test_event_cb());
}