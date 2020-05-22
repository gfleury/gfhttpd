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

void *http3_client_thread(void *arg);

int test_event_cb()
{
    struct app_context app_ctx;

    if (http3_init_config(app_ctx.config) < 0)
    {
        printf("failed to run init_http3_config(), errno: %d", errno);
        return 1;
    }

    int fds[2], ret = 0;
    if ((ret = socketpair(AF_UNIX, SOCK_DGRAM, 0, fds)) != 0)
    {
        perror("cannot create socket pair");
        return ret;
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

    pthread_t thread_id;
    int s = pthread_create(&thread_id, NULL, &http3_client_thread, &fds[1]);
    if (s != 0)
        handle_error_en(s, "pthread_create");

    struct event_base *loop = event_base_new();
    app_ctx.evbase = loop;
    struct connections *c = calloc(1, sizeof(struct connections));
    c->sock = fds[0];
    c->http_streams = NULL;

    app_ctx.conns = c;

    struct event *watcher = event_new(loop, fds[0], EV_READ | EV_PERSIST, http3_event_cb, &app_ctx);
    assert(watcher != NULL);

    if (event_add(watcher, NULL) < 0)
    {
        fprintf(stderr, "Client: Could not create/add a watcher event!\n");
        return 1;
    }

    sleep(1);

    ret = event_base_loop(loop, 0);

    close(fds[0]);
    close(fds[1]);
    return (EXIT_SUCCESS);
}

int main()
{
    return (test_event_cb());
}