#ifndef _H_CONFIG
#define _H_CONFIG

#include <uthash.h>

struct config
{
    char *cert_file;
    char *key_file;
};

struct modules
{
    char name[16];
    enum module_type
    {
        GOLANG = 0,
        RUST = 1,
    } module_type;
};

#endif