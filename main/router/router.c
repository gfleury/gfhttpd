#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http_stream/http_stream.h"

#include "routes.h"

#include "log/log.h"

/* Returns nonzero if the string |s| ends with the substring |sub| */
static int ends_with(const char *s, const char *sub)
{
    size_t slen = strlen(s);
    size_t sublen = strlen(sub);
    if (slen < sublen)
    {
        return 0;
    }
    return memcmp(s + slen - sublen, sub, sublen) == 0;
}

/* Minimum check for directory traversal. Returns nonzero if it is
   safe. */
int check_path(const char *path)
{
    /* We don't like '\' in url. */
    return path[0] && path[0] == '/' && strchr(path, '\\') == NULL &&
           strstr(path, "/../") == NULL && strstr(path, "/./") == NULL &&
           !ends_with(path, "/..") && !ends_with(path, "/.");
}

const char ERROR_HTML[] = "<html><head><title>404</title></head>"
                          "<body><h1>404 Not Found</h1></body></html>";

const char *error_html(int *size)
{
    *size = sizeof(ERROR_HTML);
    return ERROR_HTML;
}

void module_cb(int fd, short event, void *arg);

static int get_socketpair(int *socket_fds)
{
    int ret = 0;
    if ((ret = socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fds)) != 0)
    {
        log_error("Socketpair creation failed");
        return ret;
    }

    if ((ret = fcntl(socket_fds[0], F_SETFL, O_NONBLOCK)) != 0)
    {
        log_error("failed to make socket non-blocking");
        return ret;
    }
    if ((ret = fcntl(socket_fds[1], F_SETFL, O_NONBLOCK)) != 0)
    {
        log_error("failed to make socket non-blocking");
        return ret;
    }
    return ret;
}

int error_fd(struct http_stream *hs, char *http_status)
{
    int ret;
    ssize_t writelen;
    int pipefd[2];

    ret = pipe(pipefd);
    if (ret != 0)
    {
        log_error("Unable to create pipe for error handling");
        return ret;
    }

    int error_html_size = -1;
    const char *c_error_html = error_html(&error_html_size);

    writelen = write(pipefd[1], c_error_html, error_html_size - 1);
    close(pipefd[1]);

    if (writelen != error_html_size - 1)
    {
        log_error("Unable to write whole error msg, written %d / expected %d. errno: %d",
                  writelen, error_html_size, errno);
        if (writelen == -1)
        {
            return writelen;
        }
    }

    hs->response.content_lenght = writelen;
    hs->response.http_status = http_status;
    return pipefd[0];
}

int root_router(struct event_base *loop, struct http_stream *hs)
{
    int fd;
    struct route *route;
    char *rel_path, *request_path = hs->request.url;
    struct route_match rm = {
        NULL,
        NULL,
    };

    rel_path = request_path;

    // Try get specific route
    if (get_route(rel_path, &rm) == -1)
    {
        // Try to get route by regex
        match_route(rel_path, &rm);
    }

    route = rm.route;

    if (route != NULL)
    {
        int ret;
        int socket_fds[2];
        if ((ret = get_socketpair(socket_fds)) != 0)
        {
            return ret;
        }

        fd = socket_fds[0];
        hs->response.fd = socket_fds[1];

        hs->request.modules_chain = route->modules_chain;

        hs->request.modules_url = rm.stripped_path;

        struct event *module_event = evtimer_new(loop, module_cb, hs);
        struct timeval half_sec = {0, 2000};

        if ((ret = evtimer_add(module_event, &half_sec)) < 0)
        {
            log_error("Could not add a module_event event");
            return ret;
        }
    }
    // For now Always return a fd from a local file if no module match was found.
    else
    {
        // Remove root slash for local files
        for (rel_path = request_path; *rel_path == '/'; ++rel_path)
            ;

        fd = open(rel_path, O_RDONLY);
        int len = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        hs->response.content_lenght = len;
    }

    // Handle no route error as a 404
    if (fd < 0)
    {
        log_debug("No handler found for %s", rel_path);
        fd = error_fd(hs, "404");
    }

    return fd;
}
