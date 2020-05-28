#include "http2.h"
#include "http2_session.h"

#include <event2/buffer.h>

/* writecb for bufferevent. To greaceful shutdown after sending or
   receiving GOAWAY, we check the some conditions on the nghttp2
   library and output buffer of bufferevent. If it indicates we have
   no business to this session, tear down the connection. If the
   connection is not going to shutdown, we call session_send() to
   process pending data in the output buffer. This is necessary
   because we have a threshold on the buffer size to avoid too much
   buffering. See send_callback(). */
void http2_writecb(struct bufferevent *bev, void *ptr)
{
    struct http_stream *hs = (struct http_stream *)ptr;
    http2_session_data *session_data = (http2_session_data *)hs->http2_params;

    if (evbuffer_get_length(bufferevent_get_output(bev)) > 0)
    {
        return;
    }
    if (nghttp2_session_want_read(session_data->session) == 0 &&
        nghttp2_session_want_write(session_data->session) == 0)
    {
        delete_http2_session_data(hs);
        return;
    }
    if (session_send(hs) != 0)
    {
        delete_http2_session_data(hs);
        return;
    }
}
