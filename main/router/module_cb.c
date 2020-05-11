#include <errno.h>
#include <unistd.h>

#include "log/log.h"

#include "http_stream/http_stream.h"

#include "modules/golang/golang_bridge.h"

void module_cb(int fd, short event, void *arg)
{
    struct http_stream *conn_io = arg;
    int ret = -1;

    switch (conn_io->request.modules_chain->module->module_type)
    {
    case GOLANG:
        ret = Go_golang(conn_io, conn_io->request.modules_chain->module->name);
        break;
    case RUST:
        ret = -1;
        break;
    default:
        log_error("Module type not supported: %d", conn_io->request.modules_chain->module->module_type);
    }

    if (ret < 0)
    {
        log_error("Something bad happened there...");
        return;
    }
    // Check if writing was already done.
    if (conn_io->request.modules_chain->next->next != NULL)
    {
        conn_io->request.modules_chain->next = conn_io->request.modules_chain->next->next;
        conn_io->request.modules_chain->module = conn_io->request.modules_chain->next->module;
        struct event *module_event = evtimer_new(conn_io->app_ctx->evbase, module_cb, conn_io);
        struct timeval half_sec = {0, 2000};

        if (evtimer_add(module_event, &half_sec) < 0)
        {
            log_error("Could not add a module_event event");
            return;
        }
    }
}
