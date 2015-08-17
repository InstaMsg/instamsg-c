#ifndef INSTAMSG_VENDOR
#define INSTAMSG_VENDOR

#include "../../common/include/globals.h"
#include "../../common/include/instamsg_vendor_common.h"

struct Network
{
    char host[MAX_BUFFER_SIZE];
    int port;

    unsigned char socketCorrupted;

    COMMUNICATION_INTERFACE(Network)
};


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
    TIMER_INTERFACE
};


struct System
{
    SYSTEM_INTERFACE
};


#endif
