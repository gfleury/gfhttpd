#include "http2.h"
#include "http2_session.h"

/* readcb for bufferevent after client connection header was
   checked. */
void http2_readcb(struct bufferevent *bev, void *ptr)
{
    http2_session_data *session_data = (http2_session_data *)ptr;
    (void)bev;

    if (session_recv(session_data) != 0)
    {
        delete_http2_session_data(session_data);
        return;
    }
}
