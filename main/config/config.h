#ifndef _H_CONFIG
#define _H_CONFIG

struct config
{
    char cert_file[256];
    char key_file[256];
};

struct module
{
    char name[16];
    enum module_type
    {
        GOLANG = 0,
        RUST = 1,
    } module_type;
};

struct modules_chain;
struct modules_chain
{
    struct module *module;
    struct modules_chain *next;
};

int conf_load(int config_fd);
extern struct config *config;

struct config_map
{
    char *token_name;
    char *dest;
    int n_dest; /* dest lenght */
    int (*array_parser_ptr)(const char *, int);
};

#endif