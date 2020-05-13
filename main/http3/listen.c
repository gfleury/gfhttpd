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

#include "http3.h"

#include "log/log.h"

void http3_start_listen(struct event_base *evbase, const char *service,
                        app_context *app_ctx)
{
  int rv;

  struct addrinfo *res, *rp;
  struct addrinfo hints = {
      .ai_family = AF_UNSPEC,
      .ai_socktype = SOCK_DGRAM,
      .ai_protocol = IPPROTO_UDP,
#ifdef AI_ADDRCONFIG
      .ai_flags = AI_ADDRCONFIG,
#endif /* AI_ADDRCONFIG */
  };

  rv = getaddrinfo(NULL, service, &hints, &res);
  if (rv != 0)
  {
    errx(1, "Could not resolve server address");
  }
  for (rp = res; rp; rp = rp->ai_next)
  {
    int sock = http3_init_sock(rp);
    if (sock < 0)
    {
      perror("failed to run init_sock()");
      continue;
    }

    if (http3_init_config() < 0)
    {
      perror("failed to run init_http3_config()");
      continue;
    }

    struct connections *c = calloc(1, sizeof(struct connections));
    c->sock = sock;
    c->h = NULL;

    app_ctx->conns = c;
    app_ctx->evbase = evbase;

    int ret = evutil_make_socket_nonblocking(sock);
    assert(ret >= 0);

    struct event *accept_event = event_new(evbase, sock, EV_READ | EV_PERSIST, http3_event_cb, app_ctx);
    assert(accept_event != NULL);

    if (event_add(accept_event, NULL) < 0)
    {
      log_error("Could not create/add a accept event!");
      return;
    }
    else if (accept_event)
    {
      freeaddrinfo(res);
      return;
    }
  }
  log_error("Could not start listener");
}