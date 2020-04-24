#ifndef _H_LOG
#define _H_LOG

#include <event2/event-config.h>
#include <event2/event.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __AT__ __FILE__ ":" TOSTRING(__LINE__)

// https://github.com/rxi/log.c/blob/master/src/log.h
enum
{
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
};

#define log_trace(...) log_log(LOG_TRACE, __AT__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __AT__, __VA_ARGS__)
#define log_info(...) log_log(LOG_INFO, __AT__, __VA_ARGS__)
#define log_warn(...) log_log(LOG_WARN, __AT__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __AT__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __AT__, __VA_ARGS__)

struct log
{
    int fds_pipe[2];
    int level;
    struct event *evpipe;
    void (*log_func)(char *, size_t);
};

int init_log(struct event_base *loop, struct log *plog);
void destroy_log();
void log_log(int level, const char *at, const char *fmt, ...);
#endif