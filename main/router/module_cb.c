#include <errno.h>
#include <unistd.h>

#include "log/log.h"

#include "http_stream/http_stream.h"

#include "modules/golang/golang_bridge.h"

void module_cb(int fd, short event, void *arg)
{
    struct http_stream *hs = arg;
    int ret = -1;

    switch (hs->request.modules_chain->module->module_type)
    {
    case GOLANG:
        log_debug("Module %s choosen for %s", hs->request.modules_chain->module->name, hs->request.url);
        ret = Go_golang(hs, hs->request.modules_chain->module->name);
        break;
    case RUST:
        ret = -1;
        break;
    default:
        log_error("Module type not supported: %d", hs->request.modules_chain->module->module_type);
    }

    if (ret < 0)
    {
        log_error("Something bad happened there...");
        return;
    }
    // Check if writing was already done.
    if (hs->request.modules_chain->next != NULL)
    {
        hs->request.modules_chain = hs->request.modules_chain->next;
        struct event *module_event = evtimer_new(hs->app_ctx->evbase, module_cb, hs);
        struct timeval half_sec = {0, 2000};

        if (evtimer_add(module_event, &half_sec) < 0)
        {
            log_error("Could not add a module_event event");
            return;
        }
    }
}
