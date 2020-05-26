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

int http3_start_listen(struct event_base *evbase, const char *service,
                       app_context *app_ctx)
{
  int sock = http3_init_sock(app_ctx->config);
  if (sock < 0)
  {
    log_error("failed to run init_sock(), errno: %d", errno);
    return 1;
  }

  if (http3_init_config(app_ctx->config) < 0)
  {
    log_error("failed to run init_http3_config(), errno: %d", errno);
    return 1;
  }

  struct connections *c = calloc(1, sizeof(struct connections));
  c->sock = sock;

  app_ctx->conns = c;
  app_ctx->evbase = evbase;

  int ret = evutil_make_socket_nonblocking(sock);
  assert(ret >= 0);

  app_ctx->evaccept_http3 = event_new(evbase, sock, EV_READ | EV_PERSIST, http3_event_cb, app_ctx);
  assert(app_ctx->evaccept_http3 != NULL);

  if (event_add(app_ctx->evaccept_http3, NULL) < 0)
  {
    log_error("Could not create/add a accept event!");
    return 1;
  }
  else if (!app_ctx->evaccept_http3)
  {
    log_error("Could not start listener");
    return 1;
  }

  return 0;
}