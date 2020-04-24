#include <err.h>
#include <string.h>

#include "log/log.h"
#include "ssl.h"

static unsigned char next_proto_list[256];
static size_t next_proto_list_len;

#ifndef OPENSSL_NO_NEXTPROTONEG
static int next_proto_cb(SSL *ssl, const unsigned char **data,
                         unsigned int *len, void *arg)
{
    (void)ssl;
    (void)arg;

    *data = next_proto_list;
    *len = (unsigned int)next_proto_list_len;
    return SSL_TLSEXT_ERR_OK;
}
#endif /* !OPENSSL_NO_NEXTPROTONEG */

/* Create SSL_CTX. */
SSL_CTX *create_ssl_ctx(const char *key_file, const char *cert_file)
{
    SSL_CTX *ssl_ctx;
    EC_KEY *ecdh;

    ssl_ctx = SSL_CTX_new(SSLv23_server_method());
    if (!ssl_ctx)
    {
        log_error("Could not create SSL/TLS context: %s",
                  ERR_error_string(ERR_get_error(), NULL));
    }
    SSL_CTX_set_options(ssl_ctx,
                        SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
                            SSL_OP_NO_COMPRESSION |
                            SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

    ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!ecdh)
    {
        log_error("EC_KEY_new_by_curv_name failed: %s",
                  ERR_error_string(ERR_get_error(), NULL));
    }
    SSL_CTX_set_tmp_ecdh(ssl_ctx, ecdh);
    EC_KEY_free(ecdh);

    if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key_file, SSL_FILETYPE_PEM) != 1)
    {
        log_error("Could not read private key file %s", key_file);
    }
    if (SSL_CTX_use_certificate_chain_file(ssl_ctx, cert_file) != 1)
    {
        log_error("Could not read certificate file %s", cert_file);
    }

    next_proto_list_len = http2_init_proto_list(next_proto_list);

#ifndef OPENSSL_NO_NEXTPROTONEG
    SSL_CTX_set_next_protos_advertised_cb(ssl_ctx, next_proto_cb, NULL);
#endif /* !OPENSSL_NO_NEXTPROTONEG */

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
    SSL_CTX_set_alpn_select_cb(ssl_ctx, alpn_select_proto_cb, NULL);
#endif /* OPENSSL_VERSION_NUMBER >= 0x10002000L */

    return ssl_ctx;
}
