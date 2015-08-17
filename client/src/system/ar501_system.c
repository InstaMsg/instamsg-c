/*******************************************************************************
 * Contributors:
 *
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instamsg_vendor.h"

static void getManufacturer(System *system, unsigned char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "ATOLL");
}


static void getSerialNumber(System *system, unsigned char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "RANDOM-ATOLL-SERIAL");
}


static void rebootDevice(System *sys)
{
}


void init_system_utils(System *system, void *arg)
{
    system->getManufacturer = getManufacturer;
    system->getSerialNumber = getSerialNumber;
    system->rebootDevice = rebootDevice;
}


void release_system_utils(System *system)
{
}


void SYSTEM_GLOBAL_INIT()
{
    AtollrunningFrq_init();

    /*inetialise uart 0 for 115200 baud rate with interrupt module*/
    AtollUart0_Init(115200,1);

    /*send a string"UART0 TEST" to uart 0*/
    AtollUart0_StringSend("\r\nUART0 TEST\0");

    /*inetialise uart 6 for 115200 baud rate with interrupt module*/
    AtollUart6_Init(115200,1);

    /*send a string"UART6 TEST" to uart 6*/
    AtollUart6_StringSend("\r\nUART6 TEST\0");

    /*initialize the 4 digital input */
    Atoll_gpio_input_init(1,1,1,1);

    /* initialize the gsm peripharal*/
    AtollGSMperipheral_init();
}
