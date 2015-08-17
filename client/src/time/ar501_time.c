/*******************************************************************************
 * Contributors:
 *
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/

#include <stdio.h>

#include "instamsg_vendor.h"
#include "atoll_common.h"


static void getTimeIn_YYYYmmdd4HHMMSS(Timer *timer, char *buf, int maxValueLenAllowed)
{
    sprintf(buf, "YYYYmmdd4HHMMSS");
}


static void getOffset(Timer *timer, char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "19800");
}


static void startAndCountdownTimer(Timer *timer, int seconds)
{
    Mydelay(seconds * 10);
}


void init_timer(Timer *timer, void *arg)
{
    timer->getTimeIn_YYYYmmdd4HHMMSS = getTimeIn_YYYYmmdd4HHMMSS;
    timer->getOffset = getOffset;
    timer->startAndCountdownTimer = startAndCountdownTimer;
}


void release_timer(Timer* timer)
{
}
