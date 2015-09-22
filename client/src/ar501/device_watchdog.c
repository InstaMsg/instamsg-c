/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/


#include "../common/include/log.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"


extern void ResetISR(void);
static uint32_t time;

void SysTick_IntHandler(void)
{
    time--;
    if(time == 0)
    {
        error_log("Watch-Dog-Timer is RESETTING DEVICE !!!!!!");
        SysCtlReset();
    }
}


/*
 * This method initializes the watchdog-timer.
 */
void watchdog_init()
{
    SysTickIntRegister(SysTick_IntHandler);
    SysTickPeriodSet(SysCtlClockGet());
    IntMasterEnable();
    SysTickIntEnable();
}


/*
 * This method resets the watchdog-timer.
 *
 * Once this is completed, the watchdog-timer starts counting down from "n" seconds to 0.
 * Then either of the following must happen ::
 *
 * a)
 * Counter reaches 0.
 *
 * The device must then be reset/restarted.
 *
 * b)
 * "watch_dog_disable()" is called.
 *
 * In this case, the countdown-timer stops, and the device must never be reset/restarted (until the entire
 * "watch_dog_reset_and_enable" loop is repeated).
 *
 */
void watchdog_reset_and_enable(int n)
{
    time = n;
    SysTickEnable();
}


/*
 * This method disables the watchdog-timer.
 */
void watchdog_disable()
{
    SysTickDisable();
}