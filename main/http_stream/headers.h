#ifndef _H_HEADERS
#define _H_HEADERS

#include "mem/mem.h"

#ifndef uthash_malloc
#define uthash_malloc(sz) mp_alloc(mp, sz)
#endif

#ifndef uthash_free
#define uthash_free(ptr, sz) mp_fake_free(ptr)
#endif

#include "uthash.h"

typedef struct headers
{
    char *name;
    int n_name;
    char *value;
    int n_value;

    UT_hash_handle hh; /* makes this structure hashable by uthash */
} headers;

void add_header(headers **pheaders, mem_pool mp, headers *header);
headers *create_header(mem_pool mp, char *name, int n_name, char *value, int n_value);
headers *insert_header(headers **pheaders, mem_pool mp, char *name, int n_name, char *value, int n_value);
unsigned int length_header(headers **pheaders);
headers *get_header(headers **pheaders, char *name);
void delete_header(headers **pheaders, headers *del_header);
void delete_header_all(headers **pheaders);

extern headers server_header, server_status;
#endif