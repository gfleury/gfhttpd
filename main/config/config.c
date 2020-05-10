#include <fcntl.h>
#include <sys/stat.h>

#include "log/log.h"

#include "config.h"
#include "config_internal.h"

struct config *config = NULL;

int jsoneq(const char *json, int i, struct config_map *config_map)
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
    if (t[i + 1].type != JSMN_ARRAY)
    {
        log_error("Failed parsing configuration locations block isn't an array");
        return -1;
    }
    for (j = 0; j < t[i + 1].size; j++)
    {
        jsmntok_t *g = &t[i + j + 2];
        printf("\t Locations * %.*s\n", g->end - g->start, json + g->start);
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

        /* Loop over all keys of the root object */
        for (int i = 1; i < r; i++)
        {
            for (int n = 0; config_map[n].token_name != NULL; n++)
            {

                int next_token = jsoneq(buf, i, &config_map[n]);
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