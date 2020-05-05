#include "config/config.h"

struct route
{
    char *path;
    struct modules *modules[16];
};

struct modules go_example_module = {"go_example", GOLANG};

struct route routes[] = {
    {"/golang", {&go_example_module, NULL}},
};