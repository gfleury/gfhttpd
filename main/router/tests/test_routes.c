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
        .next = &m,
        .previous = NULL,
    };

    struct route *r = insert_route("/first_level", strlen("/first_level"), &m_chain, false);
    assert(r != NULL);

    struct route *route_got = get_route("/first_level");
    assert(r == route_got);

    struct route *r2 = insert_route("/second_level/.*", strlen("/second_level/.*"), &m_chain, true);
    assert(r2 != NULL);

    // Try get specific route
    route_got = get_route("/second_level/test_regex");
    assert(r2 != route_got);

    // Try to get route by regex
    route_got = match_route("/second_level/test_regex");
    assert(r2 == route_got);

    // Try to get route by regex
    route_got = match_route("/second_level/");
    assert(r2 == route_got);

    // Try to get inexisting route by regex
    route_got = match_route("/do_not_Exists/test_regex");
    assert(NULL == route_got);

    // ^/phpmyadmin(?:/(.*))?$
    struct route *r3 = insert_route("^/phpmyadmin(?:/(.*))?$", strlen("^/phpmyadmin(?:/(.*))?$"), &m_chain, true);
    assert(r3 != NULL);

    route_got = match_route("/phpmyadmin/withslash");
    assert(r3 == route_got);

    route_got = match_route("/phpmyadmin");
    assert(r3 == route_got);

    // OK
    exit(ret);
}