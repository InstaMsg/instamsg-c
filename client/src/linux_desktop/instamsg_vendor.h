#ifndef INSTAMSG_VENDOR
#define INSTAMSG_VENDOR

#include "../common/include/instamsg_vendor_common.h"
#include "../common/include/globals.h"


#ifdef FILE_SYSTEM_INTERFACE_ENABLED
#include <stdio.h>
struct FileSystem
{
    FILE *fp;
    FILE_SYSTEM_INTERFACE
};
#endif



struct ModbusCommandInterface
{
    COMMUNICATION_INTERFACE(ModbusCommandInterface)
};


struct System
{
    SYSTEM_INTERFACE
};

#endif
