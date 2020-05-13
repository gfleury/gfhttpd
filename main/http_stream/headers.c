#include "headers.h"

headers server_header = {
    .name = "server",
    .n_name = sizeof("server") - 1,
    .value = "quiche",
    .n_value = sizeof("quiche") - 1,
    .must_free = false,
};

headers server_status = {
    .name = ":status",
    .n_name = sizeof(":status") - 1,
    .value = "200",
    .n_value = sizeof("200") - 1,
    .must_free = false,
};

headers *create_header(char *name, int n_name, char *value, int n_value, bool must_free)
{
    headers *header = calloc(1, sizeof(headers));
    header->name = name;
    header->n_name = n_name;
    header->value = value;
    header->n_value = n_value;
    header->must_free = must_free;
    return header;
}

headers *insert_header(headers *pheaders, char *name, int n_name, char *value, int n_value)
{
    headers *header;

    HASH_FIND_STR(pheaders, name, header); /* header already in the hash? */
    if (header == NULL)
    {
        header = calloc(1, sizeof(headers));
        header->name = name;
        header->n_name = n_name;
        HASH_ADD_KEYPTR(hh, pheaders, header->name, header->n_name, header);
    }
    header->value = value;
    header->n_value = n_value;

    header->must_free = false;

    return header;
}

void add_header(headers *pheaders, headers *header)
{
    HASH_ADD_KEYPTR(hh, pheaders, header->name, header->n_name, header);
}

headers *get_header(headers *pheaders, char *name)
{
    headers *h;
    HASH_FIND_STR(pheaders, name, h);
    return h;
}

void delete_header(headers *pheaders, headers *del_header)
{
    HASH_DEL(pheaders, del_header);
    free(del_header);
}

void delete_header_all(headers *pheaders)
{
    headers *current, *tmp;

    HASH_ITER(hh, pheaders, current, tmp)
    {
        HASH_DEL(pheaders, current); /* delete; users advances to next */
        if (current->must_free)
        {
            free(current->name);
            free(current->value);
        }
        free(current);
    }
}

unsigned int length_header(headers *pheaders)
{
    return HASH_COUNT(pheaders);
}