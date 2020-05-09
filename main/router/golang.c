#include <errno.h>
#include <unistd.h>

#include "log/log.h"

#include "http_stream/http_stream.h"

#include "modules/golang/golang_bridge.h"

void golang_cb(int fd, short event, void *arg)
{
    struct http_stream *conn_io = arg;

    int ret = Go_golang(conn_io, conn_io->request.modules_chain->module->name);

    if (ret < 0)
    {
        log_error("Something bad happened there...");
        return;
    }

    if (conn_io->request.modules_chain->next->next != NULL)
    {
        conn_io->request.modules_chain->next = conn_io->request.modules_chain->next->next;
        conn_io->request.modules_chain->module = conn_io->request.modules_chain->next->module;
        struct event *golang_event = evtimer_new(conn_io->app_ctx->evbase, golang_cb, conn_io);
        struct timeval half_sec = {0, 2000};

        if (evtimer_add(golang_event, &half_sec) < 0)
        {
            log_error("Could not add a golang_event event");
            return;
        }
    }
}
