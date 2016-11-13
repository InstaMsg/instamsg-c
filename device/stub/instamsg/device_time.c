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
#error "Function not implemented."
}


/*
 * This method returns the minimum-delay achievable via this device.
 */
unsigned long getMinimumDelayPossibleInMicroSeconds()
{
#error "Function not implemented."
}


/*
 * This method ACTUALLY causes the current-device to go to sleep for the minimum-delay possible.
 */
void minimumDelay()
{
#error "Function not implemented."
}


/*
 * This method returns the current-tick/timestamp.
 */
unsigned long getCurrentTick()
{
#error "Function not implemented."
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
 * Syncs the system-clock.
 *
 * Returns SUCCESS on successful-syncing.
 * Else returns FAILURE.
 */
int sync_system_clock(DateParams *dateParams, unsigned long seconds)
{
    return SUCCESS;
}
