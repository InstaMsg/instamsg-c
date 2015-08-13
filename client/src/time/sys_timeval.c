/*******************************************************************************
 * Contributors:
 *
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/


#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "instamsg_vendor.h"


static void getTimeIn_YYYYmmdd4HHMMSS(Timer *timer, unsigned char *buf, int maxValueLenAllowed)
{
    time_t time_t_time;
    struct tm* tm_info;

    time(&time_t_time);
    tm_info = localtime(&time_t_time);

    strftime(buf, maxValueLenAllowed, "%Y%m%d4%H%M%S", tm_info);
}


static void getOffset(Timer *timer, unsigned char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "19800");
}


static void startAndCountdownTimer(Timer *timer, int seconds)
{
    sleep(seconds);
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
