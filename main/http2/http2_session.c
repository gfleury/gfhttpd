#include "http2.h"

#include "http2_session.h"

#include "log/log.h"

#include <string.h>
#include <unistd.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

static void add_stream(http2_session_data *session_data,
                       struct http_stream *stream_data);

void delete_http2_session_data(http2_session_data *session_data)
{
    struct http_stream *stream_data;
    SSL *ssl = bufferevent_openssl_get_ssl(session_data->bev);
    log_debug("%s disconnected\n", session_data->client_addr);
    if (ssl)
    {
        SSL_shutdown(ssl);
    }
    bufferevent_free(session_data->bev);
    nghttp2_session_del(session_data->session);
    for (stream_data = session_data->root.next; stream_data;)
    {
        struct http_stream *next = stream_data->next;
        delete_http_stream(stream_data);
        stream_data = next;
    }
    free(session_data->client_addr);
    free(session_data);
}

void delete_http_stream(struct http_stream *stream_data)
{
    if (stream_data->sock != -1)
    {
        close(stream_data->sock);
    }
    free(stream_data->request.url);
    free(stream_data);
}

struct http_stream *create_http_stream(http2_session_data *session_data, int32_t stream_id)
{
    struct http_stream *stream_data;
    stream_data = malloc(sizeof(struct http_stream));
    memset(stream_data, 0, sizeof(struct http_stream));
    stream_data->stream_id = stream_id;
    stream_data->sock = -1;
    memset(&stream_data->request, 0, sizeof(http_request));

    add_stream(session_data, stream_data);
    return stream_data;
}

static void add_stream(http2_session_data *session_data,
                       struct http_stream *stream_data)
{
    stream_data->next = session_data->root.next;
    session_data->root.next = stream_data;
    stream_data->prev = &session_data->root;
    if (stream_data->next)
    {
        stream_data->next->prev = stream_data;
    }
}

void remove_stream(http2_session_data *session_data,
                   struct http_stream *stream_data)
{
    (void)session_data;

    stream_data->prev->next = stream_data->next;
    if (stream_data->next)
    {
        stream_data->next->prev = stream_data->prev;
    }
}

/* Serialize the frame and send (or buffer) the data to
   bufferevent. */
int session_send(http2_session_data *session_data)
{
    int rv;
    rv = nghttp2_session_send(session_data->session);
    if (rv != 0)
    {
        warnx("%s: Fatal error: %s", __AT__, nghttp2_strerror(rv));
        return -1;
    }
    return 0;
}

/* Read the data in the bufferevent and feed them into nghttp2 library
   function. Invocation of nghttp2_session_mem_recv() may make
   additional pending frames, so call session_send() at the end of the
   function. */
int session_recv(http2_session_data *session_data)
{
    ssize_t readlen;
    struct evbuffer *input = bufferevent_get_input(session_data->bev);
    size_t datalen = evbuffer_get_length(input);
    unsigned char *data = evbuffer_pullup(input, -1);

    readlen = nghttp2_session_mem_recv(session_data->session, data, datalen);
    if (readlen < 0)
    {
        warnx("%s: Fatal error: %s", __AT__, nghttp2_strerror((int)readlen));
        return -1;
    }
    if (evbuffer_drain(input, (size_t)readlen) != 0)
    {
        warnx("%s: Fatal error: evbuffer_drain failed", __AT__);
        return -1;
    }
    if (session_send(session_data) != 0)
    {
        return -1;
    }
    return 0;
}