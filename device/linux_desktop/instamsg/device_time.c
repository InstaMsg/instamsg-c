/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include <time.h>

/*
 * This method does the global-level-initialization for time (if any).
 */
void init_global_timer()
{
}


/*
 * This method returns the minimum-delay achievable via this device.
 */
unsigned long getMinimumDelayPossibleInMicroSeconds()
{
    return 500;
}


/*
 * This method ACTUALLY causes the current-device to go to sleep for the minimum-delay possible.
 */
void minimumDelay()
{
    usleep(500);
}


/*
 * This method returns the current-tick/timestamp.
 */
unsigned long getCurrentTick()
{
    return time(NULL);
}
