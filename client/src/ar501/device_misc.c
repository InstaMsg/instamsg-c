#include "instamsg_vendor.h"

/*
 * Utility-function that reboots the device.
 */
void rebootDevice()
{
}


/*
 * This method does the initialization, that is needed on a global-basis.
 * All code in this method runs right at the beginning.
 */
void bootstrapInit()
{
    /*
     * Set the clocking to run directly from the crystal.
     */
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
}
