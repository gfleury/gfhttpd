#include <err.h>
#include <string.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <nghttp2/nghttp2.h>

int http2_init_proto_list(unsigned char *next_proto_list)
{
    next_proto_list[0] = NGHTTP2_PROTO_VERSION_ID_LEN;
    memcpy(&next_proto_list[1], NGHTTP2_PROTO_VERSION_ID,
           NGHTTP2_PROTO_VERSION_ID_LEN);
    int next_proto_list_len = 1 + NGHTTP2_PROTO_VERSION_ID_LEN;
    return next_proto_list_len;
}

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
int alpn_select_proto_cb(SSL *ssl, const unsigned char **out,
                         unsigned char *outlen, const unsigned char *in,
                         unsigned int inlen, void *arg)
{
    int rv;
    (void)ssl;
    (void)arg;

    rv = nghttp2_select_next_protocol((unsigned char **)out, outlen, in, inlen);

    if (rv != 1)
    {
        return SSL_TLSEXT_ERR_NOACK;
    }

    return SSL_TLSEXT_ERR_OK;
}
#endif /* OPENSSL_VERSION_NUMBER >= 0x10002000L */

/* Create SSL object */
SSL *create_ssl(SSL_CTX *ssl_ctx)
{
    SSL *ssl;
    ssl = SSL_new(ssl_ctx);
    if (!ssl)
    {
        errx(1, "Could not create SSL/TLS session object: %s",
             ERR_error_string(ERR_get_error(), NULL));
    }
    return ssl;
}