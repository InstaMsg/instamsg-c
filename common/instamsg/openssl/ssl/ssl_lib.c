/*
 * ! \file ssl/ssl_lib.c \brief Version independent SSL functions.
 */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */
/* ====================================================================
 * Copyright (c) 1998-2007 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* ====================================================================
 * Copyright 2002 Sun Microsystems, Inc. ALL RIGHTS RESERVED.
 * ECC cipher suite support in OpenSSL originally developed by
 * SUN MICROSYSTEMS, INC., and contributed to the OpenSSL project.
 */
/* ====================================================================
 * Copyright 2005 Nokia. All rights reserved.
 *
 * The portions of the attached software ("Contribution") is developed by
 * Nokia Corporation and is licensed pursuant to the OpenSSL open source
 * license.
 *
 * The Contribution, originally written by Mika Kousa and Pasi Eronen of
 * Nokia Corporation, consists of the "PSK" (Pre-Shared Key) ciphersuites
 * support (see RFC 4279) to OpenSSL.
 *
 * No patent licenses or other rights except those expressly stated in
 * the OpenSSL open source license shall be deemed granted or received
 * expressly, by implication, estoppel, or otherwise.
 *
 * No assurances are provided by Nokia that the Contribution does not
 * infringe the patent or other intellectual property rights of any third
 * party or that the license provides you with all the necessary rights
 * to make use of the Contribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. IN
 * ADDITION TO THE DISCLAIMERS INCLUDED IN THE LICENSE, NOKIA
 * SPECIFICALLY DISCLAIMS ANY LIABILITY FOR CLAIMS BROUGHT BY YOU OR ANY
 * OTHER ENTITY BASED ON INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS OR
 * OTHERWISE.
 */

#ifdef REF_CHECK
# include <assert.h>
#endif
#include <stdio.h>
#include "./ssl_locl.h"
#include <openssl/objects.h>
#include <openssl/lhash.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>
#include <openssl/ocsp.h>
#ifndef OPENSSL_NO_DH
# include <openssl/dh.h>
#endif
#ifndef OPENSSL_NO_ENGINE
# include <openssl/engine.h>
#endif

SSL_CTX *SSL_get_SSL_CTX(const SSL *ssl)
{
    return (ssl->ctx);
}

int SSL_set_session_id_context(SSL *ssl, const unsigned char *sid_ctx,
                               unsigned int sid_ctx_len)
{
    if (sid_ctx_len > SSL_MAX_SID_CTX_LENGTH) {
        SSLerr(SSL_F_SSL_SET_SESSION_ID_CONTEXT,
               SSL_R_SSL_SESSION_ID_CONTEXT_TOO_LONG);
        return 0;
    }
    ssl->sid_ctx_length = sid_ctx_len;
    memcpy(ssl->sid_ctx, sid_ctx, sid_ctx_len);

    return 1;
}

SSL *SSL_dup(SSL *s)
{
    STACK_OF(X509_NAME) *sk;
    X509_NAME *xn;
    SSL *ret;
    int i;

    if ((ret = SSL_new(SSL_get_SSL_CTX(s))) == NULL)
        return (NULL);

    ret->version = s->version;
    ret->type = s->type;
    ret->method = s->method;

    if (s->session != NULL) {
        /* This copies session-id, SSL_METHOD, sid_ctx, and 'cert' */
        SSL_copy_session_id(ret, s);
    } else {
        /*
         * No session has been established yet, so we have to expect that
         * s->cert or ret->cert will be changed later -- they should not both
         * point to the same object, and thus we can't use
         * SSL_copy_session_id.
         */

        ret->method->ssl_free(ret);
        ret->method = s->method;
        ret->method->ssl_new(ret);

        if (s->cert != NULL) {
            if (ret->cert != NULL) {
                ssl_cert_free(ret->cert);
            }
            ret->cert = ssl_cert_dup(s->cert);
            if (ret->cert == NULL)
                goto err;
        }

        SSL_set_session_id_context(ret, s->sid_ctx, s->sid_ctx_length);
    }

    ret->options = s->options;
    ret->mode = s->mode;
    SSL_set_max_cert_list(ret, SSL_get_max_cert_list(s));
    SSL_set_read_ahead(ret, SSL_get_read_ahead(s));
    ret->msg_callback = s->msg_callback;
    ret->msg_callback_arg = s->msg_callback_arg;
    SSL_set_verify(ret, SSL_get_verify_mode(s), SSL_get_verify_callback(s));
    SSL_set_verify_depth(ret, SSL_get_verify_depth(s));
    ret->generate_session_id = s->generate_session_id;

    SSL_set_info_callback(ret, SSL_get_info_callback(s));

    ret->debug = s->debug;

    /* copy app data, a little dangerous perhaps */
    if (!CRYPTO_dup_ex_data(CRYPTO_EX_INDEX_SSL, &ret->ex_data, &s->ex_data))
        goto err;

    /* setup rbio, and wbio */
    if (s->rbio != NULL) {
        if (!BIO_dup_state(s->rbio, (char *)&ret->rbio))
            goto err;
    }
    if (s->wbio != NULL) {
        if (s->wbio != s->rbio) {
            if (!BIO_dup_state(s->wbio, (char *)&ret->wbio))
                goto err;
        } else
            ret->wbio = ret->rbio;
    }
    ret->rwstate = s->rwstate;
    ret->in_handshake = s->in_handshake;
    ret->handshake_func = s->handshake_func;
    ret->server = s->server;
    ret->renegotiate = s->renegotiate;
    ret->new_session = s->new_session;
    ret->quiet_shutdown = s->quiet_shutdown;
    ret->shutdown = s->shutdown;
    ret->state = s->state;      /* SSL_dup does not really work at any state,
                                 * though */
    ret->rstate = s->rstate;
    ret->init_num = 0;          /* would have to copy ret->init_buf,
                                 * ret->init_msg, ret->init_num,
                                 * ret->init_off */
    ret->hit = s->hit;

    X509_VERIFY_PARAM_inherit(ret->param, s->param);

    /* dup the cipher_list and cipher_list_by_id stacks */
    if (s->cipher_list != NULL) {
        if ((ret->cipher_list = sk_SSL_CIPHER_dup(s->cipher_list)) == NULL)
            goto err;
    }
    if (s->cipher_list_by_id != NULL)
        if ((ret->cipher_list_by_id = sk_SSL_CIPHER_dup(s->cipher_list_by_id))
            == NULL)
            goto err;

    /* Dup the client_CA list */
    if (s->client_CA != NULL) {
        if ((sk = sk_X509_NAME_dup(s->client_CA)) == NULL)
            goto err;
        ret->client_CA = sk;
        for (i = 0; i < sk_X509_NAME_num(sk); i++) {
            xn = sk_X509_NAME_value(sk, i);
            if (sk_X509_NAME_set(sk, i, X509_NAME_dup(xn)) == NULL) {
                X509_NAME_free(xn);
                goto err;
            }
        }
    }

    if (0) {
 err:
        if (ret != NULL)
            SSL_free(ret);
        ret = NULL;
    }
    return (ret);
}

long SSL_ctrl(SSL *s, int cmd, long larg, void *parg)
{
    long l;

    switch (cmd) {
    case SSL_CTRL_GET_READ_AHEAD:
        return (s->read_ahead);
    case SSL_CTRL_SET_READ_AHEAD:
        l = s->read_ahead;
        s->read_ahead = larg;
        return (l);

    case SSL_CTRL_SET_MSG_CALLBACK_ARG:
        s->msg_callback_arg = parg;
        return 1;

    case SSL_CTRL_OPTIONS:
        return (s->options |= larg);
    case SSL_CTRL_CLEAR_OPTIONS:
        return (s->options &= ~larg);
    case SSL_CTRL_MODE:
        return (s->mode |= larg);
    case SSL_CTRL_CLEAR_MODE:
        return (s->mode &= ~larg);
    case SSL_CTRL_GET_MAX_CERT_LIST:
        return (s->max_cert_list);
    case SSL_CTRL_SET_MAX_CERT_LIST:
        l = s->max_cert_list;
        s->max_cert_list = larg;
        return (l);
    case SSL_CTRL_SET_MAX_SEND_FRAGMENT:
        if (larg < 512 || larg > SSL3_RT_MAX_PLAIN_LENGTH)
            return 0;
        s->max_send_fragment = larg;
        return 1;
    case SSL_CTRL_GET_RI_SUPPORT:
        if (s->s3)
            return s->s3->send_connection_binding;
        else
            return 0;
    case SSL_CTRL_CERT_FLAGS:
        return (s->cert->cert_flags |= larg);
    case SSL_CTRL_CLEAR_CERT_FLAGS:
        return (s->cert->cert_flags &= ~larg);

    case SSL_CTRL_GET_RAW_CIPHERLIST:
        if (parg) {
            if (s->cert->ciphers_raw == NULL)
                return 0;
            *(unsigned char **)parg = s->cert->ciphers_raw;
            return (int)s->cert->ciphers_rawlen;
        } else
            return ssl_put_cipher_by_char(s, NULL, NULL);
    default:
        return (s->method->ssl_ctrl(s, cmd, larg, parg));
    }
}

int SSL_set_ssl_method(SSL *s, const SSL_METHOD *meth)
{
    int conn = -1;
    int ret = 1;

    if (s->method != meth) {
        if (s->handshake_func != NULL)
            conn = (s->handshake_func == s->method->ssl_connect);

        if (s->method->version == meth->version)
            s->method = meth;
        else {
            s->method->ssl_free(s);
            s->method = meth;
            ret = s->method->ssl_new(s);
        }

        if (conn == 1)
            s->handshake_func = meth->ssl_connect;
        else if (conn == 0)
            s->handshake_func = meth->ssl_accept;
    }
    return (ret);
}

/*
 * Now in theory, since the calling process own 't' it should be safe to
 * modify.  We need to be able to read f without being hassled
 */
void SSL_copy_session_id(SSL *t, const SSL *f)
{
    CERT *tmp;

    /* Do we need to to SSL locking? */
    SSL_set_session(t, SSL_get_session(f));

    /*
     * what if we are setup as SSLv2 but want to talk SSLv3 or vice-versa
     */
    if (t->method != f->method) {
        t->method->ssl_free(t); /* cleanup current */
        t->method = f->method;  /* change method */
        t->method->ssl_new(t);  /* setup new */
    }

    tmp = t->cert;
    if (f->cert != NULL) {
        CRYPTO_add(&f->cert->references, 1, CRYPTO_LOCK_SSL_CERT);
        t->cert = f->cert;
    } else
        t->cert = NULL;
    if (tmp != NULL)
        ssl_cert_free(tmp);
    SSL_set_session_id_context(t, f->sid_ctx, f->sid_ctx_length);
}

SSL *SSL_new(SSL_CTX *ctx)
{
    SSL *s;

    if (ctx == NULL) {
        SSLerr(SSL_F_SSL_NEW, SSL_R_NULL_SSL_CTX);
        return (NULL);
    }
    if (ctx->method == NULL) {
        SSLerr(SSL_F_SSL_NEW, SSL_R_SSL_CTX_HAS_NO_DEFAULT_SSL_VERSION);
        return (NULL);
    }

    s = (SSL *)OPENSSL_malloc(sizeof(SSL));
    if (s == NULL)
        goto err;
    memset(s, 0, sizeof(SSL));

    s->options = ctx->options;
    s->mode = ctx->mode;
    s->max_cert_list = ctx->max_cert_list;

    if (ctx->cert != NULL) {
        /*
         * Earlier library versions used to copy the pointer to the CERT, not
         * its contents; only when setting new parameters for the per-SSL
         * copy, ssl_cert_new would be called (and the direct reference to
         * the per-SSL_CTX settings would be lost, but those still were
         * indirectly accessed for various purposes, and for that reason they
         * used to be known as s->ctx->default_cert). Now we don't look at the
         * SSL_CTX's CERT after having duplicated it once.
         */

        s->cert = ssl_cert_dup(ctx->cert);
        if (s->cert == NULL)
            goto err;
    } else
        s->cert = NULL;         /* Cannot really happen (see SSL_CTX_new) */

    s->read_ahead = ctx->read_ahead;
    s->msg_callback = ctx->msg_callback;
    s->msg_callback_arg = ctx->msg_callback_arg;
    s->verify_mode = ctx->verify_mode;
#if 0
    s->verify_depth = ctx->verify_depth;
#endif
    s->sid_ctx_length = ctx->sid_ctx_length;
    OPENSSL_assert(s->sid_ctx_length <= sizeof s->sid_ctx);
    memcpy(&s->sid_ctx, &ctx->sid_ctx, sizeof(s->sid_ctx));
    s->verify_callback = ctx->default_verify_callback;
    s->generate_session_id = ctx->generate_session_id;

    s->param = X509_VERIFY_PARAM_new();
    if (!s->param)
        goto err;
    X509_VERIFY_PARAM_inherit(s->param, ctx->param);
#if 0
    s->purpose = ctx->purpose;
    s->trust = ctx->trust;
#endif
    s->quiet_shutdown = ctx->quiet_shutdown;
    s->max_send_fragment = ctx->max_send_fragment;

    CRYPTO_add(&ctx->references, 1, CRYPTO_LOCK_SSL_CTX);
    s->ctx = ctx;
#ifndef OPENSSL_NO_TLSEXT
    s->tlsext_debug_cb = 0;
    s->tlsext_debug_arg = NULL;
    s->tlsext_ticket_expected = 0;
    s->tlsext_status_type = -1;
    s->tlsext_status_expected = 0;
    s->tlsext_ocsp_ids = NULL;
    s->tlsext_ocsp_exts = NULL;
    s->tlsext_ocsp_resp = NULL;
    s->tlsext_ocsp_resplen = -1;
    CRYPTO_add(&ctx->references, 1, CRYPTO_LOCK_SSL_CTX);
    s->initial_ctx = ctx;
# ifndef OPENSSL_NO_EC
    if (ctx->tlsext_ecpointformatlist) {
        s->tlsext_ecpointformatlist =
            BUF_memdup(ctx->tlsext_ecpointformatlist,
                       ctx->tlsext_ecpointformatlist_length);
        if (!s->tlsext_ecpointformatlist)
            goto err;
        s->tlsext_ecpointformatlist_length =
            ctx->tlsext_ecpointformatlist_length;
    }
    if (ctx->tlsext_ellipticcurvelist) {
        s->tlsext_ellipticcurvelist =
            BUF_memdup(ctx->tlsext_ellipticcurvelist,
                       ctx->tlsext_ellipticcurvelist_length);
        if (!s->tlsext_ellipticcurvelist)
            goto err;
        s->tlsext_ellipticcurvelist_length =
            ctx->tlsext_ellipticcurvelist_length;
    }
# endif
# ifndef OPENSSL_NO_NEXTPROTONEG
    s->next_proto_negotiated = NULL;
# endif

    if (s->ctx->alpn_client_proto_list) {
        s->alpn_client_proto_list =
            OPENSSL_malloc(s->ctx->alpn_client_proto_list_len);
        if (s->alpn_client_proto_list == NULL)
            goto err;
        memcpy(s->alpn_client_proto_list, s->ctx->alpn_client_proto_list,
               s->ctx->alpn_client_proto_list_len);
        s->alpn_client_proto_list_len = s->ctx->alpn_client_proto_list_len;
    }
#endif

    s->verify_result = X509_V_OK;

    s->method = ctx->method;

    if (!s->method->ssl_new(s))
        goto err;

    s->references = 1;
    s->server = (ctx->method->ssl_accept == ssl_undefined_function) ? 0 : 1;

    SSL_clear(s);

    CRYPTO_new_ex_data(CRYPTO_EX_INDEX_SSL, s, &s->ex_data);

#ifndef OPENSSL_NO_PSK
    s->psk_client_callback = ctx->psk_client_callback;
    s->psk_server_callback = ctx->psk_server_callback;
#endif

    return (s);
 err:
    if (s != NULL)
        SSL_free(s);
    SSLerr(SSL_F_SSL_NEW, ERR_R_MALLOC_FAILURE);
    return (NULL);
}

void SSL_free(SSL *s)
{
    int i;

    if (s == NULL)
        return;

    i = CRYPTO_add(&s->references, -1, CRYPTO_LOCK_SSL);
#ifdef REF_PRINT
    REF_PRINT("SSL", s);
#endif
    if (i > 0)
        return;
#ifdef REF_CHECK
    if (i < 0) {
        fprintf(stderr, "SSL_free, bad reference count\n");
        abort();                /* ok */
    }
#endif

    if (s->param)
        X509_VERIFY_PARAM_free(s->param);

    CRYPTO_free_ex_data(CRYPTO_EX_INDEX_SSL, s, &s->ex_data);

    if (s->bbio != NULL) {
        /* If the buffering BIO is in place, pop it off */
        if (s->bbio == s->wbio) {
            s->wbio = BIO_pop(s->wbio);
        }
        BIO_free(s->bbio);
        s->bbio = NULL;
    }
    if (s->rbio != NULL)
        BIO_free_all(s->rbio);
    if ((s->wbio != NULL) && (s->wbio != s->rbio))
        BIO_free_all(s->wbio);

    if (s->init_buf != NULL)
        BUF_MEM_free(s->init_buf);

    /* add extra stuff */
    if (s->cipher_list != NULL)
        sk_SSL_CIPHER_free(s->cipher_list);
    if (s->cipher_list_by_id != NULL)
        sk_SSL_CIPHER_free(s->cipher_list_by_id);

    /* Make the next call work :-) */
    if (s->session != NULL) {
        ssl_clear_bad_session(s);
        SSL_SESSION_free(s->session);
    }

    ssl_clear_cipher_ctx(s);
    ssl_clear_hash_ctx(&s->read_hash);
    ssl_clear_hash_ctx(&s->write_hash);

    if (s->cert != NULL)
        ssl_cert_free(s->cert);
    /* Free up if allocated */

#ifndef OPENSSL_NO_TLSEXT
    if (s->tlsext_hostname)
        OPENSSL_free(s->tlsext_hostname);
    if (s->initial_ctx)
        SSL_CTX_free(s->initial_ctx);
# ifndef OPENSSL_NO_EC
    if (s->tlsext_ecpointformatlist)
        OPENSSL_free(s->tlsext_ecpointformatlist);
    if (s->tlsext_ellipticcurvelist)
        OPENSSL_free(s->tlsext_ellipticcurvelist);
# endif                         /* OPENSSL_NO_EC */
    if (s->tlsext_opaque_prf_input)
        OPENSSL_free(s->tlsext_opaque_prf_input);
    if (s->tlsext_ocsp_exts)
        sk_X509_EXTENSION_pop_free(s->tlsext_ocsp_exts, X509_EXTENSION_free);
    if (s->tlsext_ocsp_ids)
        sk_OCSP_RESPID_pop_free(s->tlsext_ocsp_ids, OCSP_RESPID_free);
    if (s->tlsext_ocsp_resp)
        OPENSSL_free(s->tlsext_ocsp_resp);
    if (s->alpn_client_proto_list)
        OPENSSL_free(s->alpn_client_proto_list);
#endif

    if (s->client_CA != NULL)
        sk_X509_NAME_pop_free(s->client_CA, X509_NAME_free);

    if (s->method != NULL)
        s->method->ssl_free(s);

    if (s->ctx)
        SSL_CTX_free(s->ctx);

#ifndef OPENSSL_NO_KRB5
    if (s->kssl_ctx != NULL)
        kssl_ctx_free(s->kssl_ctx);
#endif                          /* OPENSSL_NO_KRB5 */

#if !defined(OPENSSL_NO_TLSEXT) && !defined(OPENSSL_NO_NEXTPROTONEG)
    if (s->next_proto_negotiated)
#endif

#ifndef OPENSSL_NO_SRTP
    if (s->srtp_profiles)
        sk_SRTP_PROTECTION_PROFILE_free(s->srtp_profiles);
#endif

    OPENSSL_free(s);
}

int SSL_state(const SSL *ssl)
{
    return (ssl->state);
}

void SSL_set_info_callback(SSL *ssl,
                           void (*cb) (const SSL *ssl, int type, int val))
{
    ssl->info_callback = cb;
}

/*
 * One compiler (Diab DCC) doesn't like argument names in returned function
 * pointer.
 */
void (*SSL_get_info_callback(const SSL *ssl)) (const SSL * /* ssl */ ,
                                               int /* type */ ,
                                               int /* val */ ) {
    return ssl->info_callback;
}

int SSL_get_verify_mode(const SSL *s)
{
    return (s->verify_mode);
}

int SSL_get_verify_depth(const SSL *s)
{
    return X509_VERIFY_PARAM_get_depth(s->param);
}

int (*SSL_get_verify_callback(const SSL *s)) (int, X509_STORE_CTX *) {
    return (s->verify_callback);
}

int SSL_CTX_get_verify_mode(const SSL_CTX *ctx)
{
    return (ctx->verify_mode);
}

int SSL_CTX_get_verify_depth(const SSL_CTX *ctx)
{
    return X509_VERIFY_PARAM_get_depth(ctx->param);
}

int (*SSL_CTX_get_verify_callback(const SSL_CTX *ctx)) (int, X509_STORE_CTX *) {
    return (ctx->default_verify_callback);
}

void SSL_set_verify_depth(SSL *s, int depth)
{
    X509_VERIFY_PARAM_set_depth(s->param, depth);
}

void SSL_set_read_ahead(SSL *s, int yes)
{
    s->read_ahead = yes;
}

int SSL_get_read_ahead(const SSL *s)
{
    return (s->read_ahead);
}

void ssl_clear_cipher_ctx(SSL *s)
{
    if (s->enc_read_ctx != NULL) {
        EVP_CIPHER_CTX_cleanup(s->enc_read_ctx);
        OPENSSL_free(s->enc_read_ctx);
        s->enc_read_ctx = NULL;
    }
    if (s->enc_write_ctx != NULL) {
        EVP_CIPHER_CTX_cleanup(s->enc_write_ctx);
        OPENSSL_free(s->enc_write_ctx);
        s->enc_write_ctx = NULL;
    }
#ifndef OPENSSL_NO_COMP
    if (s->expand != NULL) {
        COMP_CTX_free(s->expand);
        s->expand = NULL;
    }
    if (s->compress != NULL) {
        COMP_CTX_free(s->compress);
        s->compress = NULL;
    }
#endif
}

void ssl_clear_hash_ctx(EVP_MD_CTX **hash)
{

    if (*hash)
        EVP_MD_CTX_destroy(*hash);
    *hash = NULL;
}

int SSL_read(SSL *s, void *buf, int num)
{
    if (s->handshake_func == 0) {
        SSLerr(SSL_F_SSL_READ, SSL_R_UNINITIALIZED);
        return -1;
    }

    if (s->shutdown & SSL_RECEIVED_SHUTDOWN) {
        s->rwstate = SSL_NOTHING;
        return (0);
    }
    return (s->method->ssl_read(s, buf, num));
}

int SSL_write(SSL *s, const void *buf, int num)
{
    if (s->handshake_func == 0) {
        SSLerr(SSL_F_SSL_WRITE, SSL_R_UNINITIALIZED);
        return -1;
    }

    if (s->shutdown & SSL_SENT_SHUTDOWN) {
        s->rwstate = SSL_NOTHING;
        SSLerr(SSL_F_SSL_WRITE, SSL_R_PROTOCOL_IS_SHUTDOWN);
        return (-1);
    }
    return (s->method->ssl_write(s, buf, num));
}

int SSL_clear(SSL *s)
{

    if (s->method == NULL) {
        SSLerr(SSL_F_SSL_CLEAR, SSL_R_NO_METHOD_SPECIFIED);
        return (0);
    }

    if (ssl_clear_bad_session(s)) {
        SSL_SESSION_free(s->session);
        s->session = NULL;
    }

    s->error = 0;
    s->hit = 0;
    s->shutdown = 0;

#if 0
    /*
     * Disabled since version 1.10 of this file (early return not
     * needed because SSL_clear is not called when doing renegotiation)
     */
    /*
     * This is set if we are doing dynamic renegotiation so keep
     * the old cipher.  It is sort of a SSL_clear_lite :-)
     */
    if (s->renegotiate)
        return (1);
#else
    if (s->renegotiate) {
        SSLerr(SSL_F_SSL_CLEAR, ERR_R_INTERNAL_ERROR);
        return 0;
    }
#endif

    s->type = 0;

    s->state = SSL_ST_BEFORE | ((s->server) ? SSL_ST_ACCEPT : SSL_ST_CONNECT);

    s->version = s->method->version;
    s->client_version = s->version;
    s->rwstate = SSL_NOTHING;
    s->rstate = SSL_ST_READ_HEADER;
#if 0
    s->read_ahead = s->ctx->read_ahead;
#endif

    if (s->init_buf != NULL) {
        BUF_MEM_free(s->init_buf);
        s->init_buf = NULL;
    }

    ssl_clear_cipher_ctx(s);
    ssl_clear_hash_ctx(&s->read_hash);
    ssl_clear_hash_ctx(&s->write_hash);

    s->first_packet = 0;

#if 1
    /*
     * Check to see if we were changed into a different method, if so, revert
     * back if we are not doing session-id reuse.
     */
    if (!s->in_handshake && (s->session == NULL)
        && (s->method != s->ctx->method)) {
        s->method->ssl_free(s);
        s->method = s->ctx->method;
        if (!s->method->ssl_new(s))
            return (0);
    } else
#endif
        s->method->ssl_clear(s);
    return (1);
}

int SSL_pending(const SSL *s)
{
    /*
     * SSL_pending cannot work properly if read-ahead is enabled
     * (SSL_[CTX_]ctrl(..., SSL_CTRL_SET_READ_AHEAD, 1, NULL)), and it is
     * impossible to fix since SSL_pending cannot report errors that may be
     * observed while scanning the new data. (Note that SSL_pending() is
     * often used as a boolean value, so we'd better not return -1.)
     */
    return (s->method->ssl_pending(s));
}

int SSL_shutdown(SSL *s)
{
    /*
     * Note that this function behaves differently from what one might
     * expect.  Return values are 0 for no success (yet), 1 for success; but
     * calling it once is usually not enough, even if blocking I/O is used
     * (see ssl3_shutdown).
     */

    if (s->handshake_func == 0) {
        SSLerr(SSL_F_SSL_SHUTDOWN, SSL_R_UNINITIALIZED);
        return -1;
    }

    if ((s != NULL) && !SSL_in_init(s))
        return (s->method->ssl_shutdown(s));
    else
        return (1);
}

int SSL_renegotiate(SSL *s)
{
    if (s->renegotiate == 0)
        s->renegotiate = 1;

    s->new_session = 1;

    return (s->method->ssl_renegotiate(s));
}

/*
 * For the next 2 functions, SSL_clear() sets shutdown and so one of these
 * calls will reset it
 */
void SSL_set_accept_state(SSL *s)
{
    s->server = 1;
    s->shutdown = 0;
    s->state = SSL_ST_ACCEPT | SSL_ST_BEFORE;
    s->handshake_func = s->method->ssl_accept;
    /* clear the current cipher */
    ssl_clear_cipher_ctx(s);
    ssl_clear_hash_ctx(&s->read_hash);
    ssl_clear_hash_ctx(&s->write_hash);
}

void SSL_set_connect_state(SSL *s)
{
    s->server = 0;
    s->shutdown = 0;
    s->state = SSL_ST_CONNECT | SSL_ST_BEFORE;
    s->handshake_func = s->method->ssl_connect;
    /* clear the current cipher */
    ssl_clear_cipher_ctx(s);
    ssl_clear_hash_ctx(&s->read_hash);
    ssl_clear_hash_ctx(&s->write_hash);
}

void SSL_set_bio(SSL *s, BIO *rbio, BIO *wbio)
{
    /*
     * If the output buffering BIO is still in place, remove it
     */
    if (s->bbio != NULL) {
        if (s->wbio == s->bbio) {
            s->wbio = s->wbio->next_bio;
            s->bbio->next_bio = NULL;
        }
    }
    if ((s->rbio != NULL) && (s->rbio != rbio))
        BIO_free_all(s->rbio);
    if ((s->wbio != NULL) && (s->wbio != wbio) && (s->rbio != s->wbio))
        BIO_free_all(s->wbio);
    s->rbio = rbio;
    s->wbio = wbio;
}

BIO *SSL_get_rbio(const SSL *s)
{
    return (s->rbio);
}

BIO *SSL_get_wbio(const SSL *s)
{
    return (s->wbio);
}

