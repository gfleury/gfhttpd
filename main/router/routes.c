#include "config/config.h"

#include "routes.h"

#include "log/log.h"

struct route *routes = NULL;

static char *ROOT = "/";

pcre2_code *parse_regex(PCRE2_SPTR pattern)
{
    pcre2_code *re;
    int errornumber;
    PCRE2_SIZE erroroffset;

    re = pcre2_compile(
        pattern,               /* the pattern */
        PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
        0,                     /* default options */
        &errornumber,          /* for error number */
        &erroroffset,          /* for error offset */
        NULL);                 /* use default compile context */

    /* Compilation failed: print the error message and exit. */

    if (re == NULL)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        log_error("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,
                  buffer);
    }
    return re;
}

int match_route(char *s, struct route_match *rm)
{
    PCRE2_SPTR subject = (PCRE2_SPTR)s;
    PCRE2_SIZE subject_length;
    struct route *r, *tmp;
    int ret;

    if (!subject)
    {
        return -1;
    }
    subject_length = (PCRE2_SIZE)strlen(s);

    HASH_ITER(hh, routes, r, tmp)
    {
        if (!r->re)
        {
            // Jump non-regex routes
            continue;
        }
        pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(r->re, NULL);

        /* Now run the match. */
        ret = pcre2_match(
            r->re,          /* the compiled pattern */
            subject,        /* the subject string */
            subject_length, /* the length of the subject */
            0,              /* start at offset 0 in the subject */
            0,              /* default options */
            match_data,     /* block for storing the result */
            NULL);          /* use default match context */

        if (ret < 0)
        {
            // No match, not going to use it anymore
            pcre2_match_data_free(match_data);

            switch (ret)
            {
            // No match
            case PCRE2_ERROR_NOMATCH:
                continue;
            // Handle other special cases if you like
            default:
                log_error("PCRE2 Matching error %d", ret);
                break;
            }
            return -1;
        }
        else
        {
            // Found a match
            rm->route = r;
            PCRE2_SIZE *ovector;
            ovector = pcre2_get_ovector_pointer(match_data);
            log_debug("Match succeeded at offset %d", (int)ovector[0]);

            if (ret == 0)
            {
                log_error("Ovector was not big enough for all the captured substrings");
            }
            else
            {
                int idx = ret - 1;
                PCRE2_SPTR substring_start = subject + ovector[2 * idx];
                PCRE2_SIZE substring_length = ovector[2 * idx + 1] - ovector[2 * idx];
                log_debug("Found remaining substring - %2d: %.*s\n", ret, (int)substring_length, (char *)substring_start);

                // Exact match, no second part on path
                if (idx == 0)
                {
                    s = ROOT;
                }
                else
                {
                    // Strip found match path from full path to pass it to the modules
                    PCRE2_SIZE fullmatch_length = ovector[2 * 0 + 1] - ovector[2 * 0];
                    s += (int)(fullmatch_length - substring_length);
                    log_debug("Going with - %s", s);
                    // Find the previous slash
                    for (; *s != '/'; s--)
                        ;
                }

                rm->stripped_path = s;
            }
            ret = 0;
            break;
        }
    }

    return ret;
}

struct route *insert_route(char *path, int n_path, struct modules_chain *m, bool regex)
{
    struct route *r = NULL;

    r = (struct route *)calloc(1, sizeof *r);
    r->path = path;
    r->n_path = n_path;
    r->modules_chain = m;
    if (!regex)
    {
        r->re = NULL;
    }
    else
    {
        r->re = parse_regex((PCRE2_SPTR)path);
    }

    HASH_ADD_KEYPTR(hh, routes, r->path, r->n_path, r);

    return r;
}

struct route *create_route(char *path, int n_path, struct modules_chain *m, bool regex)
{
    struct route *r = NULL;

    r = (struct route *)calloc(1, sizeof *r);
    r->path = path;
    if (regex)
    {
        r->re = NULL;
    }
    else
    {
        r->re = parse_regex((PCRE2_SPTR)path);
    }

    return r;
}

void add_route(struct route *r)
{
    HASH_ADD_KEYPTR(hh, routes, r->path, r->n_path, r);
}

int get_route(char *path, struct route_match *rm)
{
    struct route *r;
    HASH_FIND_STR(routes, path, r);
    if (r != NULL)
    {
        rm->route = r;
        // Strip found path from path to pass it to the modules
        rm->stripped_path = ROOT;
    }
    else
    {
        return -1;
    }
    return 0;
}

void delete_route(struct route *r)
{
    HASH_DEL(routes, r);
    free(r);
}

void delete_routes_all()
{
    struct route *current, *tmp;

    HASH_ITER(hh, routes, current, tmp)
    {
        HASH_DEL(routes, current); /* delete; users advances to next */
        free(current);
    }
}

unsigned int length_routes()
{
    return HASH_COUNT(routes);
}
