#include <nghttp2/nghttp2.h>

#include "../http_stream/http_stream.h"

typedef struct http2_session_data
{
    struct bufferevent *bev;
    nghttp2_session *session;
} http2_session_data;

void delete_http_stream(struct http_stream *hs);

void delete_http2_session_data(struct http_stream *hs);

struct http_stream *create_http_stream(int32_t stream_id);

void remove_stream(http2_session_data *session_data, struct http_stream *hs);

int session_recv(struct http_stream *hs);

int session_send(struct http_stream *hs);