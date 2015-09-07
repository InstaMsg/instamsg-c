#ifndef INSTAMSG_VENDOR
#define INSTAMSG_VENDOR

#include "../../common/include/instamsg_vendor_common.h"
#include "../../common/include/globals.h"

/*
 * VERY IMPORTANT NOTE ::
 *
 * A device-implementor is free to add any additional fields/variables in the following structs, which can
 * then be accessed through the struct-pointers passed to the APIs.
 *
 */


struct Network
{
    int socket;

    char host[MAX_BUFFER_SIZE];
    int port;

    unsigned char socketCorrupted;

    COMMUNICATION_INTERFACE(Network)
};


struct SerialLoggerInterface
{
    // Feel free to add any additional fields as desired.

    int (*write)(SerialLoggerInterface *serialLoggerInterface, unsigned char* buffer, int len);
};


struct ModbusCommandInterface
{
    // Feel free to add any additional fields as desired.

    COMMUNICATION_INTERFACE(ModbusCommandInterface)
};


struct Timer
{
    // Feel free to add any additional fields as desired.

    TIMER_INTERFACE
};


struct System
{
    // Feel free to add any additional fields as desired.

    SYSTEM_INTERFACE
};

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "inc/tm4c1230d5pm.h"

#endif
