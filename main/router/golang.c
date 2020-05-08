#include <errno.h>
#include <unistd.h>

#include "log/log.h"

#include "http_stream/http_stream.h"

#include "modules/golang/golang_bridge.h"

void golang_cb(int fd, short event, void *arg)
{
    struct http_stream *conn_io = arg;

    int ret = Go_golang(conn_io, conn_io->request.modules_chain->next->name);

    if (ret < 0)
    {
        log_error("Something bad happened there...");
    }
}
