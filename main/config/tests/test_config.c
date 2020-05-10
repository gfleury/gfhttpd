#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config/config.h"

static const char *JSON_STRING =
    "{"
    "cert_file: \"file.pem\","
    "key_file: \"key.pem\","
    "locations:"
    "["
    "{'location': '/golang', 'modules': ['go_example']}"
    "]"
    "}";

int main()
{

    int r;
    int fds[2];

    if (pipe(fds) != 0)
    {
        printf("unable to create pipe\n");
        return -1;
    }

    r = write(fds[1], JSON_STRING, strlen(JSON_STRING));
    if (r != strlen(JSON_STRING))
    {
        printf("write failed");
        return -1;
    }
    printf("WRITTEN: %d\n", r);
    close(fds[1]);

    int fconf = fds[0];
    r = conf_load(fconf);
    if (r < 0)
    {
        printf("Failed to parse JSON: %d, errno: %d\n", r, errno);
        return 1;
    }

    printf("--> %s\n", config->cert_file);
    printf("--> %s\n", config->key_file);
    assert(strcmp(config->cert_file, "file.pem") == 0);
    assert(strcmp(config->key_file, "key.pem") == 0);

    return EXIT_SUCCESS;
}