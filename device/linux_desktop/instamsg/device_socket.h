/*******************************************************************************
 *
 * Copyright (c) 2014 SenseGrow, Inc.
 *
 * SenseGrow Internet of Things (IoT) Client Frameworks
 * http://www.sensegrow.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Contributors:
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/



#ifndef INSTAMSG_SOCKET
#define INSTAMSG_SOCKET

#include "../driver/include/globals.h"

#include <string.h>

#if SSL_ENABLED == 1
#include <openssl/ssl.h>
#include <openssl/bio.h>
#endif

typedef struct Socket Socket;

struct Socket
{
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */
    char host[MAX_BUFFER_SIZE];
    int port;
    char *type;

    unsigned char socketCorrupted;

#if SSL_ENABLED == 1
    SSL *ssl;

    BIO *ssl_bio;
    BIO *inter_bio;
    BIO *network_bio;
#endif

#if GSM_INTERFACE_ENABLED == 1
    char gsmApn[MAX_GSM_PROVISION_PARAM_SIZE];
    char gsmUser[MAX_GSM_PROVISION_PARAM_SIZE];
    char gsmPass[MAX_GSM_PROVISION_PARAM_SIZE];
    char gsmPin[MAX_GSM_PROVISION_PARAM_SIZE];
    char provPin[MAX_GSM_PROVISION_PARAM_SIZE];
#endif

    int (*read) (Socket *socket, unsigned char* buffer, int len, unsigned char guaranteed);
    int (*write)(Socket *socket, unsigned char* buffer, int len);
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */



    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
    int socket;
    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
};

#endif
