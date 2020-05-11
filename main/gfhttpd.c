#include "gfhttpd.h"

#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */
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

#include "log/log.h"

#include "http2/http2.h"
#include "http2/http2_session.h"

#include "http3/http3.h"

#include "ssl.h"

#include <event.h>
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>
#include <event2/listener.h>

static void initialize_app_context(app_context *app_ctx, SSL_CTX *ssl_ctx,
                                   struct event_base *evbase)
{
  memset(app_ctx, 0, sizeof(app_context));
  app_ctx->ssl_ctx = ssl_ctx;
  app_ctx->evbase = evbase;
}

static void run()
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

  ssl_ctx = create_ssl_ctx(config->key_file, config->cert_file);

  initialize_app_context(&app_ctx, ssl_ctx, evbase);

  http2_start_listen(evbase, config->listen_port, &app_ctx);
  http3_start_listen(evbase, config->listen_port, &app_ctx);

  event_base_loop(evbase, EVLOOP_NO_EXIT_ON_EMPTY);

  destroy_log(&plog);
  event_base_free(evbase);
  SSL_CTX_free(ssl_ctx);
}

void usage()
{
  printf("Usage:\n\t-v \t\tprints version and exit\n\t-c config_file\t"
         "load configuration from file (default etc/conf.json)\n\n");
}

int main(int argc, char **argv)
{
  struct sigaction act;
  char *config_file = "etc/conf.json";
  int opt, fconf;

  // put ':' in the starting of the
  // string so that program can
  //distinguish between '?' and ':'
  while ((opt = getopt(argc, argv, ":vc:")) != -1)
  {
    switch (opt)
    {
    case 'v':
      printf("gfhttpd-%s\n", GFHTTPD_VERSION);
      break;
    case 'c':
      config_file = optarg;
      break;
    case ':':
      usage();
      break;
    case '?':
    default:
      printf("unknown option: %c\n", optopt);
      usage();
      return (EXIT_FAILURE);
    }
  }

  if (optind < argc)
  {
    printf("unknow option: %s\n", argv[optind]);
    usage();
    return (EXIT_FAILURE);
  }

  fconf = open(config_file, O_RDONLY, 0);
  if (fconf < 0)
  {
    printf("Failed to open file %s for configuration reading. errno: %d\n", config_file, errno);
    return (EXIT_FAILURE);
  }

  if (conf_load(fconf) < 0)
  {
    printf("Failed to load %s configuration file. errno: %d\n", config_file, errno);
    return (EXIT_FAILURE);
  }

  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, NULL);

  SSL_load_error_strings();
  SSL_library_init();

  run();
  return (EXIT_SUCCESS);
}
