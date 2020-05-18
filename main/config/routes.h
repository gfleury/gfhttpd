#ifndef _H_ROUTES
#define _H_ROUTES
#include <stdbool.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "config.h"

#include "mem/mem.h"

#define uthash_malloc(sz) mp_alloc(mp, sz)
#define uthash_free(ptr, sz) mp_fake_free(ptr)
#include "uthash.h"

struct route
{
    char *path;
    int n_path;

    struct modules_chain *modules_chain;

    pcre2_code *re;

    UT_hash_handle hh; /* makes this structure hashable by uthash */
};

struct route_match
{
    struct route *route;
    char *stripped_path;
};

unsigned int length_routes(struct route **routes);
void delete_routes_all(struct route **routes);
void delete_route(struct route **routes, struct route *r);
void add_route(struct route **routes, mem_pool mp, struct route *r);
struct route *insert_route(struct route **routes, mem_pool mp, char *path, int n_path, struct modules_chain *m, bool regex);
struct route *create_route(struct route **routes, char *path, int n_path, struct modules_chain *m, bool regex);
int get_route(struct route **routes, char *path, struct route_match *rm);
int match_route(struct route **routes, char *subject, struct route_match *rm);
#endif