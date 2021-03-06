#ifndef INSTAMSG_SOCKET
#define INSTAMSG_SOCKET

#include "../driver/include/globals.h"

#include <string.h>

#if SSL_ENABLED == 1
#include "../../../common/instamsg/driver/include/sg_openssl/ssl.h"
#include "../../../common/instamsg/driver/include/sg_openssl/bio.h"

#endif

typedef struct SG_Socket SG_Socket;

struct SG_Socket
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
	char gsmMode[MAX_GSM_PROVISION_PARAM_SIZE];
#endif

    int (*read) (SG_Socket *socket, unsigned char* buffer, int len, unsigned char guaranteed);
    int (*write)(SG_Socket *socket, unsigned char* buffer, int len);
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */



    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
};

#endif
