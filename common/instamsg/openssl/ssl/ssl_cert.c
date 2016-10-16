/*
 * ! \file ssl/ssl_cert.c
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

#include <stdio.h>

#include "../e_os.h"
#ifndef NO_SYS_TYPES_H
# include <sys/types.h>
#endif

#include "../crypto/o_dir.h"
#include <openssl/objects.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>
#ifndef OPENSSL_NO_DH
# include <openssl/dh.h>
#endif
#include <openssl/bn.h>
#include "ssl_locl.h"

void ssl_sess_cert_free(SESS_CERT *sc)
{
    int i;

    if (sc == NULL)
        return;

    i = CRYPTO_add(&sc->references, -1, CRYPTO_LOCK_SSL_SESS_CERT);
#ifdef REF_PRINT
    REF_PRINT("SESS_CERT", sc);
#endif
    if (i > 0)
        return;
#ifdef REF_CHECK
    if (i < 0) {
        fprintf(stderr, "ssl_sess_cert_free, bad reference count\n");
        abort();                /* ok */
    }
#endif

    /* i == 0 */
    if (sc->cert_chain != NULL)
        sk_X509_pop_free(sc->cert_chain, X509_free);
    for (i = 0; i < SSL_PKEY_NUM; i++) {
        if (sc->peer_pkeys[i].x509 != NULL)
            X509_free(sc->peer_pkeys[i].x509);
#if 0                           /* We don't have the peer's private key.
                                 * These lines are just * here as a reminder
                                 * that we're still using a
                                 * not-quite-appropriate * data structure. */
        if (sc->peer_pkeys[i].privatekey != NULL)
            EVP_PKEY_free(sc->peer_pkeys[i].privatekey);
#endif
    }

#ifndef OPENSSL_NO_RSA
    if (sc->peer_rsa_tmp != NULL)
        RSA_free(sc->peer_rsa_tmp);
#endif

    OPENSSL_free(sc);
}


CERT *ssl_cert_dup(CERT *cert)
{
    CERT *ret;
    int i;

    ret = (CERT *)OPENSSL_malloc(sizeof(CERT));
    if (ret == NULL) {
        SSLerr(SSL_F_SSL_CERT_DUP, ERR_R_MALLOC_FAILURE);
        return (NULL);
    }

    memset(ret, 0, sizeof(CERT));

    ret->key = &ret->pkeys[cert->key - &cert->pkeys[0]];
    /*
     * or ret->key = ret->pkeys + (cert->key - cert->pkeys), if you find that
     * more readable
     */

    ret->valid = cert->valid;
    ret->mask_k = cert->mask_k;
    ret->mask_a = cert->mask_a;
    ret->export_mask_k = cert->export_mask_k;
    ret->export_mask_a = cert->export_mask_a;

#ifndef OPENSSL_NO_RSA
    if (cert->rsa_tmp != NULL) {
        RSA_up_ref(cert->rsa_tmp);
        ret->rsa_tmp = cert->rsa_tmp;
    }
    ret->rsa_tmp_cb = cert->rsa_tmp_cb;
#endif

    for (i = 0; i < SSL_PKEY_NUM; i++) {
        CERT_PKEY *cpk = cert->pkeys + i;
        CERT_PKEY *rpk = ret->pkeys + i;
        if (cpk->x509 != NULL) {
            rpk->x509 = cpk->x509;
            CRYPTO_add(&rpk->x509->references, 1, CRYPTO_LOCK_X509);
        }

        if (cpk->privatekey != NULL) {
            rpk->privatekey = cpk->privatekey;
            CRYPTO_add(&cpk->privatekey->references, 1, CRYPTO_LOCK_EVP_PKEY);
        }

        if (cpk->chain) {
            rpk->chain = X509_chain_up_ref(cpk->chain);
            if (!rpk->chain) {
                SSLerr(SSL_F_SSL_CERT_DUP, ERR_R_MALLOC_FAILURE);
                goto err;
            }
        }
        rpk->valid_flags = 0;
#ifndef OPENSSL_NO_TLSEXT
        if (cert->pkeys[i].serverinfo != NULL) {
            /* Just copy everything. */
            ret->pkeys[i].serverinfo =
                OPENSSL_malloc(cert->pkeys[i].serverinfo_length);
            if (ret->pkeys[i].serverinfo == NULL) {
                SSLerr(SSL_F_SSL_CERT_DUP, ERR_R_MALLOC_FAILURE);
                return NULL;
            }
            ret->pkeys[i].serverinfo_length =
                cert->pkeys[i].serverinfo_length;
            memcpy(ret->pkeys[i].serverinfo,
                   cert->pkeys[i].serverinfo,
                   cert->pkeys[i].serverinfo_length);
        }
#endif
    }
    ret->references = 1;
    /*
     * Set digests to defaults. NB: we don't copy existing values as they
     * will be set during handshake.
     */
    ssl_cert_set_default_md(ret);
    /* Peer sigalgs set to NULL as we get these from handshake too */
    ret->peer_sigalgs = NULL;
    ret->peer_sigalgslen = 0;
    /* Configured sigalgs however we copy across */

    if (cert->conf_sigalgs) {
        ret->conf_sigalgs = OPENSSL_malloc(cert->conf_sigalgslen);
        if (!ret->conf_sigalgs)
            goto err;
        memcpy(ret->conf_sigalgs, cert->conf_sigalgs, cert->conf_sigalgslen);
        ret->conf_sigalgslen = cert->conf_sigalgslen;
    } else
        ret->conf_sigalgs = NULL;

    if (cert->client_sigalgs) {
        ret->client_sigalgs = OPENSSL_malloc(cert->client_sigalgslen);
        if (!ret->client_sigalgs)
            goto err;
        memcpy(ret->client_sigalgs, cert->client_sigalgs,
               cert->client_sigalgslen);
        ret->client_sigalgslen = cert->client_sigalgslen;
    } else
        ret->client_sigalgs = NULL;
    /* Shared sigalgs also NULL */
    ret->shared_sigalgs = NULL;
    /* Copy any custom client certificate types */
    if (cert->ctypes) {
        ret->ctypes = OPENSSL_malloc(cert->ctype_num);
        if (!ret->ctypes)
            goto err;
        memcpy(ret->ctypes, cert->ctypes, cert->ctype_num);
        ret->ctype_num = cert->ctype_num;
    }

    ret->cert_flags = cert->cert_flags;

    ret->cert_cb = cert->cert_cb;
    ret->cert_cb_arg = cert->cert_cb_arg;

    if (cert->verify_store) {
        CRYPTO_add(&cert->verify_store->references, 1,
                   CRYPTO_LOCK_X509_STORE);
        ret->verify_store = cert->verify_store;
    }

    if (cert->chain_store) {
        CRYPTO_add(&cert->chain_store->references, 1, CRYPTO_LOCK_X509_STORE);
        ret->chain_store = cert->chain_store;
    }

    ret->ciphers_raw = NULL;

#ifndef OPENSSL_NO_TLSEXT
    if (!custom_exts_copy(&ret->cli_ext, &cert->cli_ext))
        goto err;
    if (!custom_exts_copy(&ret->srv_ext, &cert->srv_ext))
        goto err;
#endif

    return (ret);

#if !defined(OPENSSL_NO_DH) || !defined(OPENSSL_NO_ECDH)
 err:
#endif
#ifndef OPENSSL_NO_RSA
    if (ret->rsa_tmp != NULL)
        RSA_free(ret->rsa_tmp);
#endif

#ifndef OPENSSL_NO_TLSEXT
    custom_exts_free(&ret->cli_ext);
    custom_exts_free(&ret->srv_ext);
#endif

    ssl_cert_clear_certs(ret);

    return NULL;
}

/* Free up and clear all certificates and chains */

void ssl_cert_clear_certs(CERT *c)
{
    int i;
    if (c == NULL)
        return;
    for (i = 0; i < SSL_PKEY_NUM; i++) {
        CERT_PKEY *cpk = c->pkeys + i;
        if (cpk->x509) {
            X509_free(cpk->x509);
            cpk->x509 = NULL;
        }
        if (cpk->privatekey) {
            EVP_PKEY_free(cpk->privatekey);
            cpk->privatekey = NULL;
        }
        if (cpk->chain) {
            sk_X509_pop_free(cpk->chain, X509_free);
            cpk->chain = NULL;
        }
#ifndef OPENSSL_NO_TLSEXT
        if (cpk->serverinfo) {
            OPENSSL_free(cpk->serverinfo);
            cpk->serverinfo = NULL;
            cpk->serverinfo_length = 0;
        }
#endif
        /* Clear all flags apart from explicit sign */
        cpk->valid_flags &= CERT_PKEY_EXPLICIT_SIGN;
    }
}

