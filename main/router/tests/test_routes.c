#include <stdio.h>  /* printf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strcpy */

#include <assert.h>

#include "router/routes.h"

int main(int argc, char *argv[])
{
    int ret = 0;
    int len = length_routes();
    assert(len == 0);

    struct module m = {
        .name = "example_mod",
        .module_type = GOLANG,
    };

    struct modules_chain m_chain = {
        .module = &m,
        .next = NULL,
    };

    struct route_match rm = {
        NULL,
        NULL,
    };

    struct route *r = insert_route("/first_level", strlen("/first_level"), &m_chain, false);
    assert(r != NULL);

    ret = get_route("/first_level", &rm);
    assert(ret == 0);
    assert(rm.route == r);
    assert(strncmp(rm.stripped_path, "/", sizeof("/")) == 0);

    struct route *r2 = insert_route("^/second_level/(.*)", strlen("^/second_level/(.*)"), &m_chain, true);
    assert(r2 != NULL);

    // Try get specific route
    ret = get_route("/second_level/test_regex", &rm);
    assert(ret == -1);

    // Try to get route by regex
    ret = match_route("/second_level/test_regex", &rm);
    assert(ret == 0);
    assert(r2 == rm.route);
    assert(strncmp(rm.stripped_path, "/test_regex", sizeof("/test_regex")) == 0);

    // Try to get route by regex
    ret = match_route("/second_level/", &rm);
    assert(ret == 0);
    assert(r2 == rm.route);
    assert(strncmp(rm.stripped_path, "/", sizeof("/")) == 0);

    // Try to get inexisting route by regex
    ret = match_route("/do_not_Exists/test_regex", &rm);
    assert(ret == -1);

    // ^/phpmyadmin(?:/(.*))?$
    struct route *r3 = insert_route("^/phpmyadmin(?:/(.*))?$", strlen("^/phpmyadmin(?:/(.*))?$"), &m_chain, true);
    assert(r3 != NULL);

    ret = match_route("/phpmyadmin/whateverhappensthisshoudbetheonlyrest", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/whateverhappensthisshoudbetheonlyrest", sizeof("/whateverhappensthisshoudbetheonlyrest")) == 0);

    ret = match_route("/phpmyadmin", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/", sizeof("/")) == 0);

    ret = match_route("/phpmyadmin/withslash", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/withslash", sizeof("/withslash")) == 0);

    ret = match_route("/phpmyadmin/withslash/", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/withslash/", sizeof("/withslash/")) == 0);

    ret = match_route("/phpmyadmin/withslash/second_level", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/withslash/second_level", sizeof("/withslash/second_level")) == 0);

    ret = match_route("/phpmyadmin/withslash/second_level/", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/withslash/second_level/", sizeof("/withslash/second_level/")) == 0);

    ret = match_route("/phpmyadmin/", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/", sizeof("/")) == 0);

    ret = match_route("/../phpmyadmin/../", &rm);
    assert(ret == -1);

    ret = match_route("/phpmyadmin/../", &rm);
    assert(ret == 0);
    assert(r3 == rm.route);
    assert(strncmp(rm.stripped_path, "/../", sizeof("/../")) == 0);

    struct route *r4 = insert_route("^/no_slash_level(.*)", strlen("^/no_slash_level(.*)"), &m_chain, true);
    assert(r4 != NULL);

    ret = match_route("/no_slash_level/here/", &rm);
    assert(ret == 0);
    assert(r4 == rm.route);
    assert(strncmp(rm.stripped_path, "/here/", sizeof("/here/")) == 0);

    ret = match_route("/no_slash_level_something_else/here/", &rm);
    assert(ret == 0);
    assert(r4 == rm.route);
    assert(strncmp(rm.stripped_path, "/no_slash_level_something_else/here/", sizeof("/no_slash_level_something_else/here/")) == 0);

    // OK
    exit(EXIT_SUCCESS);
}