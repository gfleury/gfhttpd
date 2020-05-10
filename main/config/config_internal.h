#ifndef _H_CONFIG_INTERNAL
#define _H_CONFIG_INTERNAL

#include "jsmn.h"

static jsmn_parser p;
static jsmntok_t t[256]; /* We expect no more than 256 tokens/sections */

int jsoneq(const char *json, int i, struct config_map *config_map);
#endif