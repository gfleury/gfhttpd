//
// Lightly inspired/copied https://github.com/rxi/log.c/blob/master/src/log.c
//

#include <stdlib.h>

#include <assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "log.h"

static struct log *lplog = NULL;

static const char *level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

#define LOG_SIZE 1024
static void default_log_func(char *buf, size_t n)
{
    fprintf(stdout, "%.*s", (int)n, buf);
}

static void write_log_cb(const int fd, short int which, void *arg)
{
    struct log *plog = arg;
    char buf[LOG_SIZE];

    if (plog == NULL)
    {
        return;
    }

    int n = read(fd, buf, LOG_SIZE);
    while (n > 0)
    {
        // Call final
        lplog->log_func(buf, n);
        n = read(fd, buf, LOG_SIZE);
    }
}

int init_log(struct event_base *loop, struct log *plog)
{
    int ret = 0;
    if (plog == NULL)
    {
        return -1;
    }

    lplog = plog;

    // Set default logging function
    plog->log_func = &default_log_func;

#ifndef _LOG_SOCKETPAIR
    if ((ret = pipe(plog->fds_pipe)) != 0)
    {
        perror("Log pipe creation failed");
        return ret;
    }
#else
    if ((ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fds)) != 0)
    {
        perror("cannot create socket pair");
        return ret;
    }
#endif

    if ((ret = fcntl(plog->fds_pipe[0], F_SETFL, O_NONBLOCK)) != 0)
    {
        perror("failed to make log read socket non-blocking");
        return ret;
    }

    if ((ret = fcntl(plog->fds_pipe[1], F_SETFL, O_NONBLOCK)) != 0)
    {
        perror("failed to make log write socket non-blocking");
        return ret;
    }

    plog->evpipe = event_new(loop, plog->fds_pipe[0], EV_READ | EV_PERSIST, write_log_cb, plog);
    assert(plog->evpipe != NULL);

    if ((ret = event_add(plog->evpipe, NULL)) < 0)
    {
        fprintf(stderr, "Could not create/add a log pipe event!\n");
        return ret;
    }

    return ret;
}

void destroy_log(struct log *plog)
{
    event_del(plog->evpipe);
    event_free(plog->evpipe);
    close(plog->fds_pipe[0]);
    close(plog->fds_pipe[1]);
    free(plog);
}

void log_log(int level, const char *at, const char *fmt, ...)
{
    va_list args;
    char buf[16];
    int fd_printer;

    if (lplog == NULL)
    {
        fd_printer = fileno(stdout);
    }
    else if (level < lplog->level)
    {
        return;
    }
    else
    {
        fd_printer = lplog->fds_pipe[1];
    }

    time_t t = time(NULL);
    struct tm *lt = localtime(&t);

    buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
    dprintf(fd_printer, "%s %-5s %s: ", buf, level_names[level], at);
    va_start(args, fmt);
    vdprintf(fd_printer, fmt, args);
    va_end(args);
    dprintf(fd_printer, "\n");
}