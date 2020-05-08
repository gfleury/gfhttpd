#ifndef _H_ROUTER
#define _H_ROUTER

int error_html(char *html);
int check_path(const char *path);
int root_router(struct event_base *loop, struct http_stream *stream);
#endif