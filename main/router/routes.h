#ifndef _H_ROUTES
#define _H_ROUTES
#include <stdbool.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "config/config.h"
#include "uthash.h"

struct route
{
    char *path;
    int n_path;

    struct modules_chain *modules_chain;

    pcre2_code *re;

    UT_hash_handle hh; /* makes this structure hashable by uthash */
};

unsigned int length_routes();
void delete_routes_all();
void delete_route(struct route *r);
struct route *get_route(char *path);
void add_route(struct route *r);
struct route *insert_route(char *path, int n_path, struct modules_chain *m, bool regex);
struct route *create_route(char *path, int n_path, struct modules_chain *m, bool regex);
struct route *match_route(char *subject);
#endif