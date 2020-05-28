#include "http2.h"
#include "http2_session.h"

/* readcb for bufferevent after client connection header was
   checked. */
void http2_readcb(struct bufferevent *bev, void *ptr)
{
    struct http_stream *hs = (struct http_stream *)ptr;
    // http2_session_data *session_data = (http2_session_data *)hs->http2_params;
    (void)bev;

    if (session_recv(hs) != 0)
    {
        delete_http2_session_data(hs);
        return;
    }
}
