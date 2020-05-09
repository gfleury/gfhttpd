#ifndef _H_CONFIG
#define _H_CONFIG

#include <uthash.h>

struct config
{
    char *cert_file;
    char *key_file;
};

struct module
{
    char name[16];
    enum module_type
    {
        GOLANG = 0,
        RUST = 1,
    } module_type;

    UT_hash_handle hh; /* makes this structure hashable by uthash */
};

struct modules_chain;
struct modules_chain
{
    struct module *module;
    struct modules_chain *next;
};

#endif