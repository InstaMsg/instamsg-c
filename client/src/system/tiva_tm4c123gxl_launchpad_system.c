/*******************************************************************************
 * Contributors:
 *
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"



#include "instamsg_vendor.h"


/* This method fills in the string-value of device-manufacturer in "buf".
 *
 * Note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation
 * does not need to bother about that.
 *
 * Also, the device-implementation must take care that the length of manufacturer-value (including null-terminator)
 * is not greater than "maxValueLenAllowed".
 */
static void getManufacturer(System *system, char *buf, int maxValueLenAllowed)
{
}


/* This method fills in the string-value of device-serial-number in "buf".
 *
 * Note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation
 * does not need to bother about that.
 *
 * Also, the device-implementation must take care that the length of serial-number-value (including null-terminator)
 * is not greater than "maxValueLenAllowed".
 */
static void getSerialNumber(System *system, char *buf, int maxValueLenAllowed)
{
}


/*
 * This method reboots the device, thus completely re-initializing the system.
 */
static void rebootDevice(System *sys)
{
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void init_system_utils(System *system, void *arg)
{
    system->getManufacturer = getManufacturer;
    system->getSerialNumber = getSerialNumber;
    system->rebootDevice = rebootDevice;
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void release_system_utils(System *system)
{
}


/*
 * This method does the one-time system-initialization.
 * This code is called right at the beginning of InstaMsg-binary.
 */
void SYSTEM_GLOBAL_INIT()
{
    //
    // Set the clocking to run directly from the crystal.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
}
