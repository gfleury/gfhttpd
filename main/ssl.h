#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>

SSL *create_ssl(SSL_CTX *ssl_ctx);
SSL_CTX *create_ssl_ctx(const char *key_file, const char *cert_file);

// http2
int http2_init_proto_list(unsigned char *);
extern int alpn_select_proto_cb(SSL *ssl, const unsigned char **out,
                                unsigned char *outlen, const unsigned char *in,
                                unsigned int inlen, void *arg);