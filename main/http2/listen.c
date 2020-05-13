#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif /* HAVE_NETDB_H */
#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#include <ctype.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif /* HAVE_NETINET_IN_H */
#include <netinet/tcp.h>
#ifndef __sgi
#include <err.h>
#endif
#include <errno.h>
#include <string.h>

#include "http2.h"

#include "log/log.h"

void http2_start_listen(struct event_base *evbase, const char *service,
                        app_context *app_ctx)
{
    int rv;

    struct addrinfo *res, *rp;
    struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
#ifdef AI_ADDRCONFIG
        .ai_flags = AI_ADDRCONFIG | AI_PASSIVE,
#else
        .ai_flags = AI_PASSIVE,
#endif /* AI_ADDRCONFIG */
    };

    rv = getaddrinfo(NULL, service, &hints, &res);
    if (rv != 0)
    {
        errx(1, "Could not resolve server address");
    }
    for (rp = res; rp; rp = rp->ai_next)
    {
        struct evconnlistener *listener;
        listener = evconnlistener_new_bind(
            evbase, http2_acceptcb, app_ctx, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
            16, rp->ai_addr, (int)rp->ai_addrlen);

        if (listener)
        {
            freeaddrinfo(res);
            return;
        }
    }
    errx(1, "Could not start listener");
}
