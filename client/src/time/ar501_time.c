/*******************************************************************************
 * Contributors:
 *
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/

#include "instamsg_vendor.h"

/*
 * This method gets the device-time in the required format.
 *
 * For example, if the time is 3:55:46 PM on 30th July 2015, then the "buf" must be
 * written with ::
 *
 *          201507304155546
 *
 * Note that the 4 in the 9th place is hardcoded (a requirement for successful-processing at
 * the instamsg-server).
 *
 * Also, note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation
 * does not need to bother about that.
 *
 * Also, the buf will be large enough (>20 characters), so nothing to worry about buffer-overflow.
 */
static void getTimeIn_YYYYmmdd4HHMMSS(Timer *timer, char *buf)
{
}


/*
 * This method returns the timezone-offset of the device in seconds.
 *
 * For example, if the device is deployed in India, then the offset is 5 hours 30 minutes, which
 * translates to (5.5 * 60 * 60 = ) 19800 seconds, and the same must be written to "buf".
 *
 * Also, note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation
 * does not need to bother about that.
 *
 * Also, the buf will be large enough (>20 characters), so nothing to worry about buffer-overflow.
 */
static void getOffset(Timer *timer, char *buf)
{
}


/*
 * This method causes the current thread to wait for "n" seconds.
 */
static void startAndCountdownTimer(Timer *timer, int seconds)
{
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void init_timer(Timer *timer, void *arg)
{
    timer->getTimeIn_YYYYmmdd4HHMMSS = getTimeIn_YYYYmmdd4HHMMSS;
    timer->getOffset = getOffset;
    timer->startAndCountdownTimer = startAndCountdownTimer;
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void release_timer(Timer* timer)
{
}
