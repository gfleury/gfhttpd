#include "http2.h"
#include "http2_session.h"

#include "log/log.h"
#include "router/router.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

#define OUTPUT_WOULDBLOCK_THRESHOLD (1 << 16)

static void initialize_nghttp2_session(http2_session_data *session_data);

static ssize_t send_callback(nghttp2_session *session, const uint8_t *data,
                             size_t length, int flags, void *user_data);

static int on_frame_recv_callback(nghttp2_session *session,
                                  const nghttp2_frame *frame, void *user_data);

static int on_stream_close_callback(nghttp2_session *session, int32_t stream_id,
                                    uint32_t error_code, void *user_data);

static int on_header_callback(nghttp2_session *session,
                              const nghttp2_frame *frame, const uint8_t *name,
                              size_t namelen, const uint8_t *value,
                              size_t valuelen, uint8_t flags, void *user_data);

static int on_begin_headers_callback(nghttp2_session *session,
                                     const nghttp2_frame *frame,
                                     void *user_data);

static int on_request_recv(nghttp2_session *session,
                           http2_session_data *session_data,
                           struct http_stream *stream_data);

static int error_reply(nghttp2_session *session,
                       struct http_stream *stream_data);

static char *percent_decode(const uint8_t *value, size_t valuelen);

static int send_response(nghttp2_session *session, int32_t stream_id,
                         nghttp2_nv *nva, size_t nvlen, int fd);

static ssize_t file_read_callback(nghttp2_session *session, int32_t stream_id,
                                  uint8_t *buf, size_t length,
                                  uint32_t *data_flags,
                                  nghttp2_data_source *source,
                                  void *user_data);

static int send_server_connection_header(http2_session_data *session_data);

/* eventcb for bufferevent */
void http2_eventcb(struct bufferevent *bev, short events, void *ptr)
{
    http2_session_data *session_data = (http2_session_data *)ptr;
    if (events & BEV_EVENT_CONNECTED)
    {
        const unsigned char *alpn = NULL;
        unsigned int alpnlen = 0;
        SSL *ssl;
        (void)bev;

        log_debug("%s connected\n", session_data->client_addr);

        ssl = bufferevent_openssl_get_ssl(session_data->bev);

#ifndef OPENSSL_NO_NEXTPROTONEG
        SSL_get0_next_proto_negotiated(ssl, &alpn, &alpnlen);
#endif /* !OPENSSL_NO_NEXTPROTONEG */
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
        if (alpn == NULL)
        {
            SSL_get0_alpn_selected(ssl, &alpn, &alpnlen);
        }
#endif /* OPENSSL_VERSION_NUMBER >= 0x10002000L */

        if (alpn == NULL || alpnlen != 2 || memcmp("h2", alpn, 2) != 0)
        {
            log_debug("%s h2 is not negotiated\n", session_data->client_addr);
            delete_http2_session_data(session_data);
            return;
        }

        initialize_nghttp2_session(session_data);

        if (send_server_connection_header(session_data) != 0 ||
            session_send(session_data) != 0)
        {
            delete_http2_session_data(session_data);
            return;
        }

        return;
    }
    if (events & BEV_EVENT_EOF)
    {
        log_debug("%s EOF", session_data->client_addr);
    }
    else if (events & BEV_EVENT_ERROR)
    {
        log_debug("%s network error", session_data->client_addr);
    }
    else if (events & BEV_EVENT_TIMEOUT)
    {
        log_debug("%s timeout", session_data->client_addr);
    }
    delete_http2_session_data(session_data);
}

static void initialize_nghttp2_session(http2_session_data *session_data)
{
    nghttp2_session_callbacks *callbacks;

    nghttp2_session_callbacks_new(&callbacks);

    nghttp2_session_callbacks_set_send_callback(callbacks, send_callback);

    nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks,
                                                         on_frame_recv_callback);

    nghttp2_session_callbacks_set_on_stream_close_callback(
        callbacks, on_stream_close_callback);

    nghttp2_session_callbacks_set_on_header_callback(callbacks,
                                                     on_header_callback);

    nghttp2_session_callbacks_set_on_begin_headers_callback(
        callbacks, on_begin_headers_callback);

    nghttp2_session_server_new(&session_data->session, callbacks, session_data);

    nghttp2_session_callbacks_del(callbacks);
}

static ssize_t send_callback(nghttp2_session *session, const uint8_t *data,
                             size_t length, int flags, void *user_data)
{
    http2_session_data *session_data = (http2_session_data *)user_data;
    struct bufferevent *bev = session_data->bev;
    (void)session;
    (void)flags;

    /* Avoid excessive buffering in server side. */
    if (evbuffer_get_length(bufferevent_get_output(session_data->bev)) >=
        OUTPUT_WOULDBLOCK_THRESHOLD)
    {
        return NGHTTP2_ERR_WOULDBLOCK;
    }
    bufferevent_write(bev, data, length);
    return (ssize_t)length;
}

static int on_frame_recv_callback(nghttp2_session *session,
                                  const nghttp2_frame *frame, void *user_data)
{
    http2_session_data *session_data = (http2_session_data *)user_data;
    struct http_stream *stream_data;
    switch (frame->hd.type)
    {
    case NGHTTP2_DATA:
    case NGHTTP2_HEADERS:
        /* Check that the client request has finished */
        if (frame->hd.flags & NGHTTP2_FLAG_END_STREAM)
        {
            stream_data =
                nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
            /* For DATA and HEADERS frame, this callback may be called after
         on_stream_close_callback. Check that stream still alive. */
            if (!stream_data)
            {
                return 0;
            }
            return on_request_recv(session, session_data, stream_data);
        }
        break;
    default:
        break;
    }
    return 0;
}

static int on_stream_close_callback(nghttp2_session *session, int32_t stream_id,
                                    uint32_t error_code, void *user_data)
{
    http2_session_data *session_data = (http2_session_data *)user_data;
    struct http_stream *stream_data;
    (void)error_code;

    stream_data = nghttp2_session_get_stream_user_data(session, stream_id);
    if (!stream_data)
    {
        return 0;
    }
    remove_stream(session_data, stream_data);
    delete_http_stream(stream_data);
    return 0;
}

/* nghttp2_on_header_callback: Called when nghttp2 library emits
   single header name/value pair. */
static int on_header_callback(nghttp2_session *session,
                              const nghttp2_frame *frame, const uint8_t *name,
                              size_t namelen, const uint8_t *value,
                              size_t valuelen, uint8_t flags, void *user_data)
{
    struct http_stream *stream_data;
    const char PATH[] = ":path";
    const char METHOD[] = ":method";
    const char AUTHORITY[] = ":authority";
    const char SCHEME[] = ":scheme";

    (void)flags;
    (void)user_data;

    switch (frame->hd.type)
    {
    case NGHTTP2_HEADERS:
        if (frame->headers.cat != NGHTTP2_HCAT_REQUEST)
        {
            break;
        }
        stream_data =
            nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
        if (!stream_data || stream_data->request.url)
        {
            break;
        }
        else if (namelen == sizeof(PATH) - 1 && memcmp(PATH, name, namelen) == 0)
        {
            size_t j;
            for (j = 0; j < valuelen && value[j] != '?'; ++j)
                ;
            stream_data->request.url = percent_decode(value, j);
        }
        else if (namelen == sizeof(METHOD) - 1 && memcmp(METHOD, name, namelen) == 0)
        {
            size_t j;
            for (j = 0; j < valuelen && value[j] != '?'; ++j)
                ;
            stream_data->request.method = percent_decode(value, j);
        }
        else if (namelen == sizeof(AUTHORITY) - 1 && memcmp(AUTHORITY, name, namelen) == 0)
        {
            size_t j;
            for (j = 0; j < valuelen && value[j] != '?'; ++j)
                ;
            stream_data->request.authority = percent_decode(value, j);
        }
        else if (namelen == sizeof(SCHEME) - 1 && memcmp(SCHEME, name, namelen) == 0)
        {
            size_t j;
            for (j = 0; j < valuelen && value[j] != '?'; ++j)
                ;
            stream_data->request.scheme = percent_decode(value, j);
        }
        else
        {
            headers *h = create_header(strndup((char *)name, namelen), namelen, strndup((char *)value, valuelen), valuelen);
            HASH_ADD_KEYPTR(hh, stream_data->request.headers, h->name, h->n_name, h);
        }
        break;
    }
    return 0;
}

static int on_begin_headers_callback(nghttp2_session *session,
                                     const nghttp2_frame *frame,
                                     void *user_data)
{
    http2_session_data *session_data = (http2_session_data *)user_data;
    struct http_stream *stream_data;

    if (frame->hd.type != NGHTTP2_HEADERS ||
        frame->headers.cat != NGHTTP2_HCAT_REQUEST)
    {
        return 0;
    }
    stream_data = create_http_stream(session_data, frame->hd.stream_id);
    nghttp2_session_set_stream_user_data(session, frame->hd.stream_id,
                                         stream_data);
    return 0;
}

static int on_request_recv(nghttp2_session *session,
                           http2_session_data *session_data,
                           struct http_stream *stream_data)
{
    int fd;
    nghttp2_nv hdrs[] = {MAKE_NV(":status", "200")};

    if (!stream_data->request.url)
    {
        if (error_reply(session, stream_data) != 0)
        {
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        return 0;
    }

    log_debug("%s %s %s\n", stream_data->request.method, session_data->client_addr,
              stream_data->request.url);
    if (!check_path(stream_data->request.url))
    {
        if (error_reply(session, stream_data) != 0)
        {
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        return 0;
    }

    fd = root_router(session_data->app_ctx->evbase, stream_data);
    if (fd == -1)
    {
        if (error_reply(session, stream_data) != 0)
        {
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        return 0;
    }
    stream_data->sock = fd;

    if (send_response(session, stream_data->stream_id, hdrs, ARRLEN(hdrs), fd) !=
        0)
    {
        close(fd);
        return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    return 0;
}

static int error_reply(nghttp2_session *session,
                       struct http_stream *stream_data)
{
    int rv;
    ssize_t writelen;
    int pipefd[2];
    nghttp2_nv hdrs[] = {MAKE_NV(":status", "404")};

    rv = pipe(pipefd);
    if (rv != 0)
    {
        warn("Could not create pipe");
        rv = nghttp2_submit_rst_stream(session, NGHTTP2_FLAG_NONE,
                                       stream_data->stream_id,
                                       NGHTTP2_INTERNAL_ERROR);
        if (rv != 0)
        {
            warnx("%s: Fatal error: %s", __AT__, nghttp2_strerror(rv));
            return -1;
        }
        return 0;
    }

    char *c_error_html = NULL;
    int error_html_size = error_html(c_error_html);

    writelen = write(pipefd[1], c_error_html, error_html_size - 1);
    close(pipefd[1]);

    if (writelen != error_html_size - 1)
    {
        close(pipefd[0]);
        return -1;
    }

    stream_data->sock = pipefd[0];

    if (send_response(session, stream_data->stream_id, hdrs, ARRLEN(hdrs),
                      pipefd[0]) != 0)
    {
        close(pipefd[0]);
        return -1;
    }
    return 0;
}

/* Returns int value of hex string character |c| */
static uint8_t hex_to_uint(uint8_t c)
{
    if ('0' <= c && c <= '9')
    {
        return (uint8_t)(c - '0');
    }
    if ('A' <= c && c <= 'F')
    {
        return (uint8_t)(c - 'A' + 10);
    }
    if ('a' <= c && c <= 'f')
    {
        return (uint8_t)(c - 'a' + 10);
    }
    return 0;
}

/* Decodes percent-encoded byte string |value| with length |valuelen|
   and returns the decoded byte string in allocated buffer. The return
   value is NULL terminated. The caller must free the returned
   string. */
static char *percent_decode(const uint8_t *value, size_t valuelen)
{
    char *res;

    res = malloc(valuelen + 1);
    if (valuelen > 3)
    {
        size_t i, j;
        for (i = 0, j = 0; i < valuelen - 2;)
        {
            if (value[i] != '%' || !isxdigit(value[i + 1]) ||
                !isxdigit(value[i + 2]))
            {
                res[j++] = (char)value[i++];
                continue;
            }
            res[j++] =
                (char)((hex_to_uint(value[i + 1]) << 4) + hex_to_uint(value[i + 2]));
            i += 3;
        }
        memcpy(&res[j], &value[i], 2);
        res[j + 2] = '\0';
    }
    else
    {
        memcpy(res, value, valuelen);
        res[valuelen] = '\0';
    }
    return res;
}

static int send_response(nghttp2_session *session, int32_t stream_id,
                         nghttp2_nv *nva, size_t nvlen, int fd)
{
    int rv;
    nghttp2_data_provider data_prd;
    data_prd.source.fd = fd;
    data_prd.read_callback = file_read_callback;

    rv = nghttp2_submit_response(session, stream_id, nva, nvlen, &data_prd);
    if (rv != 0)
    {
        warnx("%s: Fatal error: %s", __AT__, nghttp2_strerror(rv));
        return -1;
    }
    return 0;
}

static ssize_t file_read_callback(nghttp2_session *session, int32_t stream_id,
                                  uint8_t *buf, size_t length,
                                  uint32_t *data_flags,
                                  nghttp2_data_source *source,
                                  void *user_data)
{
    int fd = source->fd;
    ssize_t r;
    (void)session;
    (void)stream_id;
    (void)user_data;

    while ((r = read(fd, buf, length)) == -1 && errno == EINTR)
        ;

    if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        log_error("read error: %s", strerror(errno));
        return 0;
    }
    else if (r == -1)
    {
        log_error("read error: %s", strerror(errno));
        return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
    }
    else if (r == 0 && (errno != EAGAIN || errno != EWOULDBLOCK))
    {
        log_error("read error: %s", strerror(errno));
        *data_flags |= NGHTTP2_DATA_FLAG_EOF;
    }
    return r;
}

/* Send HTTP/2 client connection header, which includes 24 bytes
   magic octets and SETTINGS frame */
static int send_server_connection_header(http2_session_data *session_data)
{
    nghttp2_settings_entry iv[1] = {
        {NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 100}};
    int rv;

    rv = nghttp2_submit_settings(session_data->session, NGHTTP2_FLAG_NONE, iv,
                                 ARRLEN(iv));
    if (rv != 0)
    {
        warnx("%s: Fatal error: %s", __AT__, nghttp2_strerror(rv));
        return -1;
    }
    return 0;
}