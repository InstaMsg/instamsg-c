/* ssl/ssl_sess.c */
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
 * Copyright (c) 1998-2006 The OpenSSL Project.  All rights reserved.
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

#include <stdio.h>
#include <openssl/lhash.h>
#include <openssl/rand.h>
#ifndef OPENSSL_NO_ENGINE
# include <openssl/engine.h>
#endif
#include "ssl_locl.h"

static int remove_session_lock(SSL_CTX *ctx, SSL_SESSION *c, int lck);

SSL_SESSION *SSL_get_session(const SSL *ssl)
/* aka SSL_get0_session; gets 0 objects, just returns a copy of the pointer */
{
    return (ssl->session);
}

int SSL_set_session(SSL *s, SSL_SESSION *session)
{
    int ret = 0;
    const SSL_METHOD *meth;

    if (session != NULL) {
        meth = s->ctx->method->get_ssl_method(session->ssl_version);
        if (meth == NULL)
            meth = s->method->get_ssl_method(session->ssl_version);
        if (meth == NULL) {
            SSLerr(SSL_F_SSL_SET_SESSION, SSL_R_UNABLE_TO_FIND_SSL_METHOD);
            return (0);
        }

        if (meth != s->method) {
            if (!SSL_set_ssl_method(s, meth))
                return (0);
        }
#ifndef OPENSSL_NO_KRB5
        if (s->kssl_ctx && !s->kssl_ctx->client_princ &&
            session->krb5_client_princ_len > 0) {
            s->kssl_ctx->client_princ =
                (char *)OPENSSL_malloc(session->krb5_client_princ_len + 1);
            memcpy(s->kssl_ctx->client_princ, session->krb5_client_princ,
                   session->krb5_client_princ_len);
            s->kssl_ctx->client_princ[session->krb5_client_princ_len] = '\0';
        }
#endif                          /* OPENSSL_NO_KRB5 */

        /* CRYPTO_w_lock(CRYPTO_LOCK_SSL); */
        CRYPTO_add(&session->references, 1, CRYPTO_LOCK_SSL_SESSION);
        if (s->session != NULL)
            SSL_SESSION_free(s->session);
        s->session = session;
        s->verify_result = s->session->verify_result;
        /* CRYPTO_w_unlock(CRYPTO_LOCK_SSL); */
        ret = 1;
    } else {
        if (s->session != NULL) {
            SSL_SESSION_free(s->session);
            s->session = NULL;
        }

        meth = s->ctx->method;
        if (meth != s->method) {
            if (!SSL_set_ssl_method(s, meth))
                return (0);
        }
        ret = 1;
    }
    return (ret);
}

int ssl_clear_bad_session(SSL *s)
{
    if ((s->session != NULL) &&
        !(s->shutdown & SSL_SENT_SHUTDOWN) &&
        !(SSL_in_init(s) || SSL_in_before(s))) {
        SSL_CTX_remove_session(s->ctx, s->session);
        return (1);
    } else
        return (0);
}

int SSL_CTX_remove_session(SSL_CTX *ctx, SSL_SESSION *c)
{
    return remove_session_lock(ctx, c, 1);
}

static int remove_session_lock(SSL_CTX *ctx, SSL_SESSION *c, int lck)
{
    SSL_SESSION *r;
    int ret = 0;

    if ((c != NULL) && (c->session_id_length != 0)) {
        if (lck)
            CRYPTO_w_lock(CRYPTO_LOCK_SSL_CTX);
        if ((r = lh_SSL_SESSION_retrieve(ctx->sessions, c)) == c) {
            ret = 1;
            r = lh_SSL_SESSION_delete(ctx->sessions, c);
            SSL_SESSION_list_remove(ctx, c);
        }

        if (lck)
            CRYPTO_w_unlock(CRYPTO_LOCK_SSL_CTX);

        if (ret) {
            r->not_resumable = 1;
            if (ctx->remove_session_cb != NULL)
                ctx->remove_session_cb(ctx, r);
            SSL_SESSION_free(r);
        }
    } else
        ret = 0;
    return (ret);
}

void SSL_SESSION_free(SSL_SESSION *ss)
{
    int i;

    if (ss == NULL)
        return;

    i = CRYPTO_add(&ss->references, -1, CRYPTO_LOCK_SSL_SESSION);
#ifdef REF_PRINT
    REF_PRINT("SSL_SESSION", ss);
#endif
    if (i > 0)
        return;
#ifdef REF_CHECK
    if (i < 0) {
        fprintf(stderr, "SSL_SESSION_free, bad reference count\n");
        abort();                /* ok */
    }
#endif

    CRYPTO_free_ex_data(CRYPTO_EX_INDEX_SSL_SESSION, ss, &ss->ex_data);

    OPENSSL_cleanse(ss->key_arg, sizeof ss->key_arg);
    OPENSSL_cleanse(ss->master_key, sizeof ss->master_key);
    OPENSSL_cleanse(ss->session_id, sizeof ss->session_id);
    if (ss->sess_cert != NULL)
        ssl_sess_cert_free(ss->sess_cert);
    if (ss->peer != NULL)
        X509_free(ss->peer);
    if (ss->ciphers != NULL)
        sk_SSL_CIPHER_free(ss->ciphers);
#ifndef OPENSSL_NO_TLSEXT
    if (ss->tlsext_hostname != NULL)
        OPENSSL_free(ss->tlsext_hostname);
    if (ss->tlsext_tick != NULL)
        OPENSSL_free(ss->tlsext_tick);
# ifndef OPENSSL_NO_EC
    ss->tlsext_ecpointformatlist_length = 0;
    if (ss->tlsext_ecpointformatlist != NULL)
        OPENSSL_free(ss->tlsext_ecpointformatlist);
    ss->tlsext_ellipticcurvelist_length = 0;
    if (ss->tlsext_ellipticcurvelist != NULL)
        OPENSSL_free(ss->tlsext_ellipticcurvelist);
# endif                         /* OPENSSL_NO_EC */
#endif
#ifndef OPENSSL_NO_PSK
    if (ss->psk_identity_hint != NULL)
        OPENSSL_free(ss->psk_identity_hint);
    if (ss->psk_identity != NULL)
        OPENSSL_free(ss->psk_identity);
#endif
#ifndef OPENSSL_NO_SRP
    if (ss->srp_username != NULL)
        OPENSSL_free(ss->srp_username);
#endif
    OPENSSL_cleanse(ss, sizeof(*ss));
    OPENSSL_free(ss);
}

