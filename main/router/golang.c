#include <errno.h>
#include <unistd.h>

#include "log/log.h"

#include "http_stream/http_stream.h"

#include "golang_bridge.h"

void golang_cb(int fd, short event, void *arg)
{
    struct http_stream *conn_io = arg;

    int ret = Go_golang(conn_io, Get_address());

    if (ret < 0)
    {
        log_error("Something bad happened there...");
    }
}
