#ifndef INSTAMSG_VENDOR
#define INSTAMSG_VENDOR

#include "../../common/include/instamsg_vendor_common.h"
#include "../../common/include/globals.h"

struct Network
{
    int socket;

    char host[MAX_BUFFER_SIZE];
    int port;

    unsigned char socketCorrupted;

    COMMUNICATION_INTERFACE(Network)
};


#ifdef FILE_SYSTEM_INTERFACE_ENABLED
#include <stdio.h>
struct FileSystem
{
    FILE *fp;
    FILE_SYSTEM_INTERFACE
};
#endif


struct SerialLoggerInterface
{
    int (*write)(SerialLoggerInterface *serialLoggerInterface, unsigned char* buffer, int len);
};


struct ModbusCommandInterface
{
    COMMUNICATION_INTERFACE(ModbusCommandInterface)
};


#include <sys/time.h>
struct Timer
{
    struct timeval end_time;
    TIMER_INTERFACE
};


struct System
{
    SYSTEM_INTERFACE
};

#endif
