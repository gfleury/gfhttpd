#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "log/log.h"

#include "config.h"
#include "config_internal.h"

#include "router/routes.h"

struct config *config = NULL;

static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if ((tok->type == JSMN_STRING || tok->type == JSMN_PRIMITIVE) && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0)
    {
        return 0;
    }
    return -1;
}

static int json_contains(const char *json, int i, struct config_map *config_map)
{
    jsmntok_t *tok = &t[i];
    if (tok->type == JSMN_PRIMITIVE && (int)strlen(config_map->token_name) == tok->end - tok->start &&
        strncmp(json + tok->start, config_map->token_name, tok->end - tok->start) == 0)
    {
        if (config_map->dest != NULL && config_map->n_dest > 0)
        {
            snprintf(config_map->dest, config_map->n_dest, "%.*s", t[i + 1].end - t[i + 1].start,
                     json + t[i + 1].start);
            return 1;
        }
        else if (config_map->array_parser_ptr != NULL)
        {
            return config_map->array_parser_ptr(json, i);
        }
    }
    return -1;
}

static int locations_parser(const char *json, int i)
{
    int j;
    jsmntok_t *locations = &t[i + 1];

    if (locations->type != JSMN_ARRAY)
    {
        log_error("Failed parsing configuration locations block isn't an array");
        return -1;
    }
    for (j = 0; j < locations->size; j++)
    {
        jsmntok_t *location = locations + j + 1;
        log_debug("Location with %d objets * %.*s", location->size,
                  location->end - location->start, json + location->start);

        if (location->type == JSMN_OBJECT)
        {
            struct route *r = calloc(1, sizeof(struct route));
            for (int h = 0; h <= location->size; h++)
            {
                jsmntok_t *loc_keys = location + 1 + h;
                if (jsoneq(json, loc_keys, "location") == 0)
                {
                    h++;
                    loc_keys += 1;
                    log_debug("Location is = %.*s", loc_keys->end - loc_keys->start,
                              json + loc_keys->start);

                    r->path = strndup(json + loc_keys->start, loc_keys->end - loc_keys->start);
                    r->n_path = loc_keys->end - loc_keys->start;
                }
                else if (jsoneq(json, loc_keys, "modules") == 0)
                {
                    struct modules_chain *previous = NULL, *first = NULL;
                    h++;
                    loc_keys += 1;
                    log_debug("Modules has %d elements and is = %.*s", loc_keys->size,
                              loc_keys->end - loc_keys->start, json + loc_keys->start);
                    for (int m_idx = 0; m_idx < loc_keys->size; m_idx++)
                    {
                        jsmntok_t *modules_keys = loc_keys + 1 + m_idx;
                        struct modules_chain *m_chain = calloc(1, sizeof(struct modules_chain));
                        if (first == NULL)
                        {
                            first = m_chain;
                        }

                        m_chain->module = calloc(1, sizeof(struct module));

                        m_chain->module->module_type = GOLANG;
                        snprintf(m_chain->module->name, sizeof(m_chain->module->name), "%.*s",
                                 modules_keys->end - modules_keys->start, json + modules_keys->start);

                        if (previous != NULL)
                        {
                            previous->next = m_chain;
                            previous = m_chain;
                        }
                        else
                        {
                            previous = m_chain;
                        }
                    }
                    r->modules_chain = first;
                }
            }
            add_route(r);
        }
    }
    i += t[i + 1].size + 1;
    return i;
}

int conf_load(int config_fd)
{
    char buf[BUFSIZ];
    jsmntok_t *ptok = NULL;
    int tokcount = 0, r = 0;
    FILE *cfile = fdopen(config_fd, "r");

    if (cfile == NULL)
    {
        return -1;
    }

    if (config == NULL)
    {
        config = calloc(1, sizeof(struct config));
    }

    struct config_map config_map[] = {
        {
            "cert_file",
            config->cert_file,
            sizeof(config->cert_file),
            NULL,
        },
        {
            "key_file",
            config->key_file,
            sizeof(config->key_file),
            NULL,
        },
        {
            "locations",
            NULL,
            0,
            &locations_parser,
        },
        {NULL, NULL, 0, NULL},
    };

    jsmn_init(&p);

    for (;;)
    {
        /* Read another chunk */
        r = fread(buf, sizeof(buf[0]), sizeof(buf), cfile);
        if (r < 0)
        {
            log_error("Unable to read from configfile: %d, errno=%d", r, errno);
            break;
        }
        else if (r == 0)
        {
            // EOF
            log_debug("EOF Config file");
            break;
        }

        log_debug("Config read: %.*s, %d", r, buf, r);

        ptok = t + (tokcount * sizeof(t[0]));
        r = jsmn_parse(&p, buf, r, ptok, (sizeof(t) / sizeof(t[0])) - tokcount);
        if (r < 0)
        {
            if (r == JSMN_ERROR_NOMEM)
            {
                log_error("Parsing memory failure, probably too much tokens on configuration file.");
                break;
            }
        }
        tokcount += r;

        /* Loop over all keys of the root object */
        for (int i = 1; i < r; i++)
        {
            for (int n = 0; config_map[n].token_name != NULL; n++)
            {

                int next_token = json_contains(buf, i, &config_map[n]);
                if (next_token > 0)
                {
                    log_debug("Setting %s with %.*s", config_map[n].token_name, t[i + 1].end - t[i + 1].start, buf + t[i + 1].start);
                    i += next_token;
                    break;
                }
            }
        }
    }

    fclose(cfile);
    return r;
}