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

#include "gfhttpd.h"
#include "log/log.h"

#include "http2/http2.h"
#include "http2/http2_session.h"

#include "http3/http3.h"

#include "ssl.h"

#include <event.h>
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>
#include <event2/listener.h>

static void http2_start_listen(struct event_base *evbase, const char *service,
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

static void http3_start_listen(struct event_base *evbase, const char *service,
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
      log_error("Could not create/add a accept event!\n");
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

static void initialize_app_context(app_context *app_ctx, SSL_CTX *ssl_ctx,
                                   struct event_base *evbase)
{
  memset(app_ctx, 0, sizeof(app_context));
  app_ctx->ssl_ctx = ssl_ctx;
  app_ctx->evbase = evbase;
}

static void run(const char *service, const char *key_file,
                const char *cert_file)
{
  SSL_CTX *ssl_ctx;
  app_context app_ctx;
  struct event_base *evbase;
  struct log plog;

  evbase = event_base_new();
  plog.level = 0;
  if (init_log(evbase, &plog) != 0)
  {
    perror("Failed to init log system.");
    return;
  }

  log_info("Logging initialized, starting gfhttpd-%s", GFHTTPD_VERSION);

  ssl_ctx = create_ssl_ctx(key_file, cert_file);

  initialize_app_context(&app_ctx, ssl_ctx, evbase);

  http2_start_listen(evbase, service, &app_ctx);
  http3_start_listen(evbase, service, &app_ctx);

  event_base_loop(evbase, EVLOOP_NO_EXIT_ON_EMPTY);

  destroy_log(&plog);
  event_base_free(evbase);
  SSL_CTX_free(ssl_ctx);
}

int main(int argc, char **argv)
{
  struct sigaction act;

  if (argc < 4)
  {
    fprintf(stderr, "Usage: libevent-server PORT KEY_FILE CERT_FILE\n");
    exit(EXIT_FAILURE);
  }

  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, NULL);

  SSL_load_error_strings();
  SSL_library_init();

  run(argv[1], argv[2], argv[3]);
  return 0;
}
