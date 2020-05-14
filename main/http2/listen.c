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

int http2_start_listen(struct event_base *evbase, const char *service,
                       app_context *app_ctx)
{

    struct sockaddr_in6 sin6;

#ifdef SIN6_LEN
    sin6.sin6_len = sizeof(sin6);
#endif
    sin6.sin6_family = AF_INET6;
    sin6.sin6_flowinfo = 0;
    sin6.sin6_port = htons(atoi(service));
    sin6.sin6_addr = in6addr_any;

    struct evconnlistener *listener;
    listener = evconnlistener_new_bind(
        evbase, http2_acceptcb, app_ctx, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
        16, (struct sockaddr *)&sin6, sizeof(sin6));

    if (!listener)
    {
        log_error("Could not start listener");
        return 1;
    }

    return 0;
}
