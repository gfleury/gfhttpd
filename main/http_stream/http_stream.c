#include "http_stream.h"

static struct http_stream *head = NULL;

struct http_stream *connections_iter()
{
    return head;
}

void add_connection(struct http_stream *hs)
{
    struct http_stream *ptr;
    ptr = get_connection(hs->cid);

    if (ptr == NULL)
    {
        HASH_ADD(hh, head, cid, LOCAL_CONN_ID_LEN, hs);
    }
    else
    {
        *ptr = *hs;
    }
}

struct http_stream *get_connection(uint8_t cid[LOCAL_CONN_ID_LEN])
{
    struct http_stream *hs;
    HASH_FIND(hh, head, cid, LOCAL_CONN_ID_LEN, hs);
    return hs;
}

void delete_connection(struct http_stream *hs)
{
    HASH_DEL(head, hs);
}

void delete_connections_all()
{
    struct http_stream *current, *tmp;
    HASH_ITER(hh, head, current, tmp)
    {
        HASH_DEL(head, current); /* delete; users advances to next */
    }
}

unsigned int length_connection()
{
    return HASH_COUNT(head);
}