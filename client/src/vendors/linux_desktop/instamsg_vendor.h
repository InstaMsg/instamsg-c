#ifndef INSTAMSG_VENDOR
#define INSTAMSG_VENDOR

#include "../../common/include/globals.h"
#include "../../common/include/instamsg_vendor_common.h"

struct Network
{
    int socket;

    unsigned char host[MAX_BUFFER_SIZE];
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
    COMMUNICATION_INTERFACE(SerialLoggerInterface)
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

#define SYSTEM_GLOBAL_INIT  ;   // Nothing to be done as such for Linux-Desktop

#endif
