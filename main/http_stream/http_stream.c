#include "http_stream.h"

void add_connection(struct http_stream **pconnections, mem_pool mp, struct http_stream *hs)
{
    HASH_ADD(hh, *pconnections, cid, sizeof(hs->cid), hs);
}

struct http_stream *get_connection(struct http_stream **pconnections, uint8_t cid[LOCAL_CONN_ID_LEN])
{
    struct http_stream *hs;
    HASH_FIND(hh, *pconnections, cid, sizeof(hs->cid), hs);
    return hs;
}

void delete_connection(struct http_stream **pconnections, struct http_stream *hs)
{
    HASH_DEL(*pconnections, hs);
}

void delete_connections_all(struct http_stream **pconnections)
{
    struct http_stream *current, *tmp;

    HASH_ITER(hh, *pconnections, current, tmp)
    {
        HASH_DEL(*pconnections, current); /* delete; users advances to next */
    }
}

unsigned int length_connection(struct http_stream **pconnections)
{
    return HASH_COUNT(*pconnections);
}