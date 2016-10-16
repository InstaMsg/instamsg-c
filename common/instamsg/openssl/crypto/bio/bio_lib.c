/* crypto/bio/bio_lib.c */
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

#include <stdio.h>
#include <errno.h>
#include <openssl/crypto.h>
#include "../cryptlib.h"
#include <openssl/bio.h>
#include <openssl/stack.h>

BIO *BIO_new(BIO_METHOD *method)
{
    BIO *ret = NULL;

    ret = (BIO *)OPENSSL_malloc(sizeof(BIO));
    if (ret == NULL) {
        BIOerr(BIO_F_BIO_NEW, ERR_R_MALLOC_FAILURE);
        return (NULL);
    }
    if (!BIO_set(ret, method)) {
        OPENSSL_free(ret);
        ret = NULL;
    }
    return (ret);
}

int BIO_set(BIO *bio, BIO_METHOD *method)
{
    bio->method = method;
    bio->callback = NULL;
    bio->cb_arg = NULL;
    bio->init = 0;
    bio->shutdown = 1;
    bio->flags = 0;
    bio->retry_reason = 0;
    bio->num = 0;
    bio->ptr = NULL;
    bio->prev_bio = NULL;
    bio->next_bio = NULL;
    bio->references = 1;
    bio->num_read = 0L;
    bio->num_write = 0L;
    CRYPTO_new_ex_data(CRYPTO_EX_INDEX_BIO, bio, &bio->ex_data);
    if (method->create != NULL)
        if (!method->create(bio)) {
            CRYPTO_free_ex_data(CRYPTO_EX_INDEX_BIO, bio, &bio->ex_data);
            return (0);
        }
    return (1);
}

int BIO_free(BIO *a)
{
    int i;

    if (a == NULL)
        return (0);

    i = CRYPTO_add(&a->references, -1, CRYPTO_LOCK_BIO);
#ifdef REF_PRINT
    REF_PRINT("BIO", a);
#endif
    if (i > 0)
        return (1);
#ifdef REF_CHECK
    if (i < 0) {
        fprintf(stderr, "BIO_free, bad reference count\n");
        abort();
    }
#endif
    if ((a->callback != NULL) &&
        ((i = (int)a->callback(a, BIO_CB_FREE, NULL, 0, 0L, 1L)) <= 0))
        return (i);

    CRYPTO_free_ex_data(CRYPTO_EX_INDEX_BIO, a, &a->ex_data);

    if ((a->method != NULL) && (a->method->destroy != NULL))
        a->method->destroy(a);
    OPENSSL_free(a);
    return (1);
}

void BIO_set_flags(BIO *b, int flags)
{
    b->flags |= flags;
}

int BIO_read(BIO *b, void *out, int outl)
{
    int i;
    long (*cb) (BIO *, int, const char *, int, long, long);

    if ((b == NULL) || (b->method == NULL) || (b->method->bread == NULL)) {
        BIOerr(BIO_F_BIO_READ, BIO_R_UNSUPPORTED_METHOD);
        return (-2);
    }

    cb = b->callback;
    if ((cb != NULL) &&
        ((i = (int)cb(b, BIO_CB_READ, out, outl, 0L, 1L)) <= 0))
        return (i);

    if (!b->init) {
        BIOerr(BIO_F_BIO_READ, BIO_R_UNINITIALIZED);
        return (-2);
    }

    i = b->method->bread(b, out, outl);

    if (i > 0)
        b->num_read += (unsigned long)i;

    if (cb != NULL)
        i = (int)cb(b, BIO_CB_READ | BIO_CB_RETURN, out, outl, 0L, (long)i);
    return (i);
}

int BIO_write(BIO *b, const void *in, int inl)
{
    int i;
    long (*cb) (BIO *, int, const char *, int, long, long);

    if (b == NULL)
        return (0);

    cb = b->callback;
    if ((b->method == NULL) || (b->method->bwrite == NULL)) {
        BIOerr(BIO_F_BIO_WRITE, BIO_R_UNSUPPORTED_METHOD);
        return (-2);
    }

    if ((cb != NULL) &&
        ((i = (int)cb(b, BIO_CB_WRITE, in, inl, 0L, 1L)) <= 0))
        return (i);

    if (!b->init) {
        BIOerr(BIO_F_BIO_WRITE, BIO_R_UNINITIALIZED);
        return (-2);
    }

    i = b->method->bwrite(b, in, inl);

    if (i > 0)
        b->num_write += (unsigned long)i;

    if (cb != NULL)
        i = (int)cb(b, BIO_CB_WRITE | BIO_CB_RETURN, in, inl, 0L, (long)i);
    return (i);
}

int BIO_test_flags(const BIO *b, int flags)
{
    return (b->flags & flags);
}

long BIO_ctrl(BIO *b, int cmd, long larg, void *parg)
{
    long ret;
    long (*cb) (BIO *, int, const char *, int, long, long);

    if (b == NULL)
        return (0);

    if ((b->method == NULL) || (b->method->ctrl == NULL)) {
        BIOerr(BIO_F_BIO_CTRL, BIO_R_UNSUPPORTED_METHOD);
        return (-2);
    }

    cb = b->callback;

    if ((cb != NULL) &&
        ((ret = cb(b, BIO_CB_CTRL, parg, cmd, larg, 1L)) <= 0))
        return (ret);

    ret = b->method->ctrl(b, cmd, larg, parg);

    if (cb != NULL)
        ret = cb(b, BIO_CB_CTRL | BIO_CB_RETURN, parg, cmd, larg, ret);
    return (ret);
}

void BIO_clear_flags(BIO *b, int flags)
{
    b->flags &= ~flags;
}

void BIO_free_all(BIO *bio)
{
    BIO *b;
    int ref;

    while (bio != NULL) {
        b = bio;
        ref = b->references;
        bio = bio->next_bio;
        BIO_free(b);
        /* Since ref count > 1, don't free anyone else. */
        if (ref > 1)
            break;
    }
}

IMPLEMENT_STACK_OF(BIO)
