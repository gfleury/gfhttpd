#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config/config.h"

#include "router/routes.h"

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
    printf("WRITTEN: %d\n", r);
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
    int fconf = create_mock_conf(JSON_STRING);
    r = conf_load(fconf);
    if (r < 0)
    {
        printf("Failed to parse JSON: %d, errno: %d\n", r, errno);
        return (EXIT_FAILURE);
    }

    close(fconf);

    assert(strcmp(config->cert_file, "file.pem") == 0);
    assert(strcmp(config->key_file, "key.pem") == 0);

    struct route *rr = get_route("/golang");
    assert(rr != NULL);

    assert(rr->modules_chain->module != NULL);

    assert(rr->modules_chain->module->module_type == GOLANG);

    assert(strncmp(rr->modules_chain->module->name, "go_example", sizeof("go_example")) == 0);

    assert(strncmp(rr->modules_chain->next->module->name, "unexistent_modu", sizeof("unexistent_modu")) == 0);
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
    int fconf = create_mock_conf(BAD_JSON_STRING);
    r = conf_load(fconf);
    close(fconf);
    assert(r < 0);

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
    int fconf = create_mock_conf(MISSING_BAD_JSON_STRING);
    r = conf_load(fconf);
    close(fconf);
    assert(r < 0);

    return (EXIT_SUCCESS);
}

int main()
{
    return test_load_conf() + test_load_broken_conf() + test_load_missing_item_conf();
}