/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "../driver/include/socket.h"
#include "../driver/include/time.h"


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
    return 0;
}


/*
 * This method ACTUALLY causes the current-device to go to sleep for the minimum-delay possible.
 */
void minimumDelay()
{
}


/*
 * This method returns the current-tick/timestamp.
 *
 * Note that the value returned by this method is used on a "relative basis" since the device reset.
 * So, this method may return either of following ::
 *
 *      * Either the absolute timestamp (as returned by the linux-command "date +%s"), or
 *      * Either the number of seconds that have passed (at the time of calling this method) since device reset.
 */
unsigned long getCurrentTick()
{
    return 0;
}


#if GSM_TIME_SYNC_PRESENT == 1
/*
 * Returns the current-timestamp, the original of which was returned via GSM.
 * Returns 0 in case no informaton is received from GSM (yet).
 */
unsigned long get_GSM_timestamp()
{
#error "Function not implemented."
}
#endif


/*
 * Syncs the system-clock, using the structure "DateParams" as defined in common/instamsg/driver/include/time.h
 *
 * At the time of calling this method the variable "dateParams" is already filled in by the appropriate values
 * of the current time in UTC.
 *
 * Returns SUCCESS on successful-syncing.
 * Else returns FAILURE.
 */
int sync_system_clock(DateParams *dateParams, unsigned long seconds)
{
    return FAILURE;
}


/*
 * This method gets the time in the following-format
 *
 *          YYYYMMDD4HHMMSS
 *          201507304155546
 *
 * The time can be in any timezone (the timzone is taken care of appriately by "getTimezoneOffset" method).
 */
void getTimeInDesiredFormat(char *buffer, int maxBufferLength)
{
}


/*
 * This method gets the timezone-offset for this device.
 */
void getTimezoneOffset(char *buffer, int maxBufferLength)
{
}
