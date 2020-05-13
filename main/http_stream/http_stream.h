#ifndef _H_HTTP_STREAM
#define _H_HTTP_STREAM

#include <sys/socket.h>

#include <openssl/ssl.h>

#include "headers.h"

#include "config/config.h"

#include <uthash.h>

#define LOCAL_CONN_ID_LEN 16

typedef struct app_context
{
    SSL_CTX *ssl_ctx;
    struct event_base *evbase;
    struct connections *conns;
} app_context;

typedef struct http_request
{
    char *url;
    char *modules_url;
    char *method;
    char *authority;
    char *scheme;
    headers *headers;
    int fd;
    size_t content_lenght;
    struct modules_chain *modules_chain;
} http_request;

typedef struct http_response
{
    char *http_status;
    headers *headers;
    int headers_sent;
    int fd;
    size_t content_lenght;
} http_response;

struct http_stream
{
    struct event *timeout_ev;
    struct timeval timer;

    int sock;
    struct app_context *app_ctx;

    // HTTP3 specific
    uint8_t cid[LOCAL_CONN_ID_LEN];
    void *http3_params;

    // HTTP2 specific
    int32_t stream_id;
    struct http_stream *next;
    struct http_stream *prev;

    http_request request;
    http_response response;

    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;

    UT_hash_handle hh;
};

struct connections
{
    int sock;
    struct http_stream *h;
};
#endif