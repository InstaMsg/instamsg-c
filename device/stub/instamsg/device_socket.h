#ifndef INSTAMSG_SOCKET
#define INSTAMSG_SOCKET

#include "../driver/include/globals.h"

#include <string.h>

typedef struct Socket Socket;

struct Socket
{
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */
    char host[MAX_BUFFER_SIZE];
    int port;

    unsigned char socketCorrupted;

#ifdef GSM_INTERFACE_ENABLED
    char gsmApn[MAX_GSM_PROVISION_PARAM_SIZE];
    char gsmUser[MAX_GSM_PROVISION_PARAM_SIZE];
    char gsmPass[MAX_GSM_PROVISION_PARAM_SIZE];
#endif

    int (*read) (Socket *socket, unsigned char* buffer, int len, unsigned char guaranteed);
    int (*write)(Socket *socket, unsigned char* buffer, int len);
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */



    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
};

#endif
