#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

#include "../driver/include/globals.h"

/*
 * Utility-function that reboots the device.
 */
void rebootDevice()
{
    startAndCountdownTimer(3, 1); /* Sleep a little, so that the above log is printed completely .. */
    SysCtlReset();
}


/*
 * This method does the initialization, that is needed on a global-basis.
 * All code in this method runs right at the beginning (followed by logger-initialization).
 */
void bootstrapInit()
{
    /*
     * Set the clocking to run directly from the crystal.
     */
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
}
