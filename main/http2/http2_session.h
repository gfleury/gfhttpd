#include <nghttp2/nghttp2.h>

#include "../http_stream/http_stream.h"

typedef struct http2_session_data
{
    struct http_stream root;
    struct bufferevent *bev;
    app_context *app_ctx;
    nghttp2_session *session;
    char *client_addr;
} http2_session_data;

void delete_http_stream(struct http_stream *stream_data);

void delete_http2_session_data(http2_session_data *session_data);

struct http_stream *create_http_stream(http2_session_data *session_data, int32_t stream_id);

void remove_stream(http2_session_data *session_data, struct http_stream *stream_data);

int session_recv(http2_session_data *session_data);

int session_send(http2_session_data *session_data);