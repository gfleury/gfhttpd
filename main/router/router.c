#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

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

int error_html(const char *html)
{
    html = ERROR_HTML;
    return sizeof(ERROR_HTML);
}

int root_router(char *request_path)
{
    char *rel_path;
    for (rel_path = request_path; *rel_path == '/'; ++rel_path)
        ;
    int fd = open(rel_path, O_RDONLY);

    return fd;
}