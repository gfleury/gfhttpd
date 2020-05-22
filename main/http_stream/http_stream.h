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
    struct config *config;
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
    struct http_stream *head;
    struct event_base *evbase;

    struct event *timeout_ev, *module_cb_ev;
    struct timeval timer;
    struct route *routes;

    mem_pool mp;

    int sock;

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
    struct http_stream *http_streams;
};

void add_connection(struct http_stream **pconnections, mem_pool mp, struct http_stream *hs);
struct http_stream *get_connection(struct http_stream **pconnections, uint8_t *cid);
void delete_connection(struct http_stream **pconnections, struct http_stream *hs);
void delete_connections_all(struct http_stream **pconnections);
unsigned int length_connection(struct http_stream **pconnections);
#endif