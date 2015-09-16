/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "./device_time.h"

/*
 * This method MUST connect the underlying medium (even if it means to retry continuously).
 */
void connect_underlying_time_medium_guaranteed(Timer *timer)
{
}


/*
 * This method returns the minimum-delay achievable via this device.
 */
unsigned int getMinimumDelayPossibleInMicroSeconds(Timer *timer)
{
    return 500;
}


/*
 * This method ACTUALLY causes the current-device to go to sleep for the minimum-delay possible.
 */
void minimumDelay(Timer *timer)
{
    usleep(500);
}


/*
 * This method MUST release the underlying medium (even if it means to retry continuously).
 */
void release_underlying_time_medium_guaranteed(Timer *timer)
{
}
