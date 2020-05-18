#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config/config.h"

#include "config/routes.h"

int create_mock_conf(const char *c)
{
    int r;
    int fds[2];

    if (pipe(fds) != 0)
    {
        printf("unable to create pipe\n");
        return (EXIT_FAILURE);
    }

    r = write(fds[1], c, strlen(c));
    if (r != strlen(c))
    {
        printf("write failed");
        return (EXIT_FAILURE);
    }
    // printf("WRITTEN: %d\n", r);
    close(fds[1]);
    return fds[0];
}

static const char *JSON_STRING =
    "{"
    "listen_port: \"file.pem\","
    "cert_file: \"file.pem\","
    "key_file: \"key.pem\","
    "locations:"
    "["
    "{location: \"/golang\", modules: [\"go_example\", \"unexistent_module\"]}"
    "]"
    "}";

int test_load_conf()
{

    int r;
    struct config config = {
        .routes = NULL,
        .mp = mp_new(2048),
    };

    int fconf = create_mock_conf(JSON_STRING);
    r = conf_load(fconf, &config);
    if (r < 0)
    {
        printf("Failed to parse JSON: %d, errno: %d\n", r, errno);
        return (EXIT_FAILURE);
    }

    close(fconf);

    assert(strcmp(config.cert_file, "file.pem") == 0);
    assert(strcmp(config.key_file, "key.pem") == 0);

    struct route_match rm = {
        NULL,
        NULL,
    };

    r = get_route(&config.routes, "/golang", &rm);
    assert(r == 0);
    assert(rm.route != NULL);

    assert(rm.route->modules_chain->module != NULL);

    assert(rm.route->modules_chain->module->module_type == GOLANG);

    assert(strncmp(rm.route->modules_chain->module->name, "go_example", sizeof("go_example")) == 0);

    assert(strncmp(rm.route->modules_chain->next->module->name, "unexistent_modu", sizeof("unexistent_modu")) == 0);
    config_free(&config);

    return (EXIT_SUCCESS);
}

static const char *BAD_JSON_STRING =
    "{"
    "listen_port: \"file.pem\","
    "cert_file: \"file.pem\","
    "key_file: \"key.pem\","
    "locations:"
    "["
    "{location: \"/golang\", xxsmodules: [\"go_example\", \"unexistent_module\"]}"
    "]"
    "}";

int test_load_broken_conf()
{
    int r;
    struct config config = {.routes = NULL, .mp = NULL};
    int fconf = create_mock_conf(BAD_JSON_STRING);
    r = conf_load(fconf, &config);
    close(fconf);
    assert(r < 0);
    config_free(&config);

    return (EXIT_SUCCESS);
}

static const char *MISSING_BAD_JSON_STRING =
    "{"
    "key_file: \"key.pem\","
    "locations:"
    "["
    "{location: \"/golang\", modules: [\"go_example\", \"unexistent_module\"]}"
    "]"
    "}";

int test_load_missing_item_conf()
{
    int r;
    struct config config = {.routes = NULL, .mp = NULL};
    int fconf = create_mock_conf(MISSING_BAD_JSON_STRING);
    r = conf_load(fconf, &config);
    close(fconf);
    assert(r < 0);
    config_free(&config);

    return (EXIT_SUCCESS);
}

int test_routes()
{
    int ret = 0;

    struct route *routes = NULL;
    mem_pool mp = mp_new(16 * 1024);

    int len = length_routes(&routes);
    assert(len == 0);

    struct module *m = mp_calloc(mp, 1, sizeof(struct module));
    snprintf(m->name, sizeof(m->name), "example_mod");
    m->module_type = GOLANG;

    struct modules_chain *m_chain = mp_calloc(mp, 1, sizeof(struct modules_chain));
    m_chain->module = m;
    m_chain->next = NULL;

    struct route_match rm = {
        NULL,
        NULL,
    };

    struct route *r = insert_route(&routes, mp, strdup("/first_level"), strlen("/first_level"), m_chain, false);
    assert(r != NULL);

    ret = get_route(&routes, "/first_level", &rm);
    assert(ret == 0);
    assert(rm.route == r);
    assert(strncmp(rm.stripped_path, "/", sizeof("/")) == 0);

    m = mp_calloc(mp, 1, sizeof(struct module));
    snprintf(m->name, sizeof(m->name), "example_mod");
    m->module_type = GOLANG;
    m_chain = mp_calloc(mp, 1, sizeof(struct modules_chain));
    m_chain->module = m;
    m_chain->next = NULL;

    struct route *r2 = insert_route(&routes, mp, strdup("^/second_level/(.*)"), strlen("^/second_level/(.*)"), m_chain, true);
    assert(r2 != NULL);

    // Try get specific route
    ret = get_route(&routes, "/second_level/test_regex", &rm);
    assert(ret == -1);

    // Try to get route by regex
    ret = match_route(&routes, "/second_level/test_regex", &rm);
    assert(ret == 0);
    assert(r2 == rm.route);
    assert(strncmp(rm.stripped_path, "/test_regex", sizeof("/test_regex")) == 0);

    // Try to get route by regex
    ret = match_route(&routes, "/second_level/", &rm);
    assert(ret == 0);
    assert(r2 == rm.route);
    assert(strncmp(rm.stripped_path, "/", sizeof("/")) == 0);

    // Try to get inexisting route by regex
    ret = match_route(&routes, "/do_not_Exists/test_regex", &rm);
    assert(ret == -1);

    m = mp_calloc(mp, 1, sizeof(struct module));
    snprintf(m->name, sizeof(m->name), "example_mod");
    m->module_type = GOLANG;
    m_chain = mp_calloc(mp, 1, sizeof(struct modules_chain));
    m_chain->module = m;
    m_chain->next = NULL;

    // ^/phpmyadmin(?:/(.*))?$
    struct route *r3 = insert_route(&routes, mp, strdup("^/phpmyadmin(?:/(.*))?$"), strlen("^/phpmyadmin(?:/(.*))?$"), m_chain, true);
    assert(r3 != NULL);

    ret = match_route(&routes, "/phpmyadmin/whateverhappensthisshoudbetheonlyrest", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/whateverhappensthisshoudbetheonlyrest", sizeof("/whateverhappensthisshoudbetheonlyrest")) == 0);

    ret = match_route(&routes, "/phpmyadmin", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/", sizeof("/")) == 0);

    ret = match_route(&routes, "/phpmyadmin/withslash", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/withslash", sizeof("/withslash")) == 0);

    ret = match_route(&routes, "/phpmyadmin/withslash/", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/withslash/", sizeof("/withslash/")) == 0);

    ret = match_route(&routes, "/phpmyadmin/withslash/second_level", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/withslash/second_level", sizeof("/withslash/second_level")) == 0);

    ret = match_route(&routes, "/phpmyadmin/withslash/second_level/", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/withslash/second_level/", sizeof("/withslash/second_level/")) == 0);

    ret = match_route(&routes, "/phpmyadmin/", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/", sizeof("/")) == 0);

    ret = match_route(&routes, "/../phpmyadmin/../", &rm);
    assert(ret == -1);

    ret = match_route(&routes, "/phpmyadmin/../", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/../", sizeof("/../")) == 0);

    m = mp_calloc(mp, 1, sizeof(struct module));
    snprintf(m->name, sizeof(m->name), "example_mod");
    m->module_type = GOLANG;
    m_chain = mp_calloc(mp, 1, sizeof(struct modules_chain));
    m_chain->module = m;
    m_chain->next = NULL;

    struct route *r4 = insert_route(&routes, mp, strdup("^/no_slash_level(.*)"), strlen("^/no_slash_level(.*)"), m_chain, true);
    assert(r4 != NULL);

    ret = match_route(&routes, "/no_slash_level/here/", &rm);
    assert(ret == 0);
    assert(r4 == rm.route);
    assert(strncmp(rm.stripped_path, "/here/", sizeof("/here/")) == 0);

    ret = match_route(&routes, "/no_slash_level_something_else/here/", &rm);
    assert(ret == 0);
    assert(r4 == rm.route);
    assert(strncmp(rm.stripped_path, "/no_slash_level_something_else/here/", sizeof("/no_slash_level_something_else/here/")) == 0);

    delete_routes_all(&routes);
    mp_delete(&mp, true);
    // OK
    return (EXIT_SUCCESS);
}

int main()
{
    return test_routes() + test_load_conf() + test_load_broken_conf() + test_load_missing_item_conf();
}