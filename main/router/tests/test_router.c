#include <stdio.h>  /* printf */
#include <stdlib.h> /* malloc */
#include <string.h> /* strcpy */
#include <unistd.h>

#include <assert.h>

#include <event2/event-config.h>
#include <event2/event.h>

#include "config/routes.h"

#include "http_stream/http_stream.h"

#include "router/router.h"

int main()
{
    struct event_base *evbase = event_base_new();
    assert(evbase);

    // Mock http_stream
    struct http_stream hs;
    hs.request.url = strdup("/example_mod");
    hs.request.method = strdup("GET");

    // Mock Routes
    struct route *routes = NULL;
    mem_pool mp = mp_new(16 * 1024);

    int len = length_routes(&routes);
    assert(len == 0);

    struct module *m = mp_calloc(mp, 1, sizeof(struct module));
    snprintf(m->name, sizeof(m->name), "go_example");
    m->module_type = GOLANG;

    struct modules_chain *m_chain = mp_calloc(mp, 1, sizeof(struct modules_chain));
    m_chain->module = m;
    m_chain->next = NULL;

    struct route *r = insert_route(&routes, mp, strdup("/example_mod"), strlen("/example_mod"), m_chain, false);
    assert(r != NULL);

    int fd = root_router(evbase, routes, &hs);
    assert(fd > 0);

    int ret = event_base_loop(evbase, 0);
    assert(ret == 1); // 1 = no more events to run

    char buf[1024];

    while ((ret = read(fd, &buf, sizeof(buf))) > 0)
    {
        assert(strncmp(buf, "Hello, \"/\" map[]\n", ret) == 0);
    }
    event_base_free(evbase);
    delete_routes_all(&routes);
    mp_delete(&mp, true);

    return (EXIT_SUCCESS);
}