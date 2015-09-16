#ifndef INSTAMSG_NETWORK
#define INSTAMSG_NETWORK

#include "../common/include/globals.h"

#include <string.h>

typedef struct Network Network;

struct Network
{
    char host[MAX_BUFFER_SIZE];
    int port;

    unsigned char socketCorrupted;

    /*
     * Any extra fields may be added here.
     */

    int (*read) (Network *network, unsigned char* buffer, int len, unsigned char guaranteed);
    int (*write)(Network *network, unsigned char* buffer, int len);
};

#endif


