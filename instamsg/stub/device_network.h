#ifndef INSTAMSG_NETWORK
#define INSTAMSG_NETWORK

#include "../common/include/globals.h"

#include <string.h>

typedef struct Network Network;

struct Network
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

    int (*read) (Network *network, unsigned char* buffer, int len, unsigned char guaranteed);
    int (*write)(Network *network, unsigned char* buffer, int len);
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */



    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
};

#endif
