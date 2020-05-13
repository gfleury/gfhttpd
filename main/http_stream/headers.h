#ifndef _H_HEADERS
#define _H_HEADERS
#include "uthash.h"

#include <stdbool.h>

typedef struct headers
{
    char *name;
    int n_name;
    char *value;
    int n_value;
    bool must_free;

    UT_hash_handle hh; /* makes this structure hashable by uthash */
} headers;

void add_header(headers *pheaders, headers *header);
headers *create_header(char *name, int n_name, char *value, int n_value, bool must_free);
headers *insert_header(headers *pheaders, char *name, int n_name, char *value, int n_value);
unsigned int length_header(headers *pheaders);
headers *get_header(headers *pheaders, char *name);
void delete_header(headers *pheaders, headers *del_header);
void delete_header_all(headers *pheaders);

extern headers server_header, server_status;
#endif