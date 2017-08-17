/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "../driver/include/watchdog.h"

/*
 * This method initializes the watchdog-timer.
 */
void init_watchdog()
{
}


/*
 * This method resets the watchdog-timer.
 *
 * n         : Number of seconds the watchdog should wait for.
 *
 * immediate : 0/1
 *             Denotes whether the device should be reset/rebooted immediately.
 *
 */
void do_watchdog_reset_and_enable(int n, unsigned char immediate)
{
}


/*
 * This method disables the watchdog-timer.
 */
void do_watchdog_disable()
{
}
