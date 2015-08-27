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


static void getTimeIn_YYYYmmdd4HHMMSS(Timer *timer, char *buf)
{
    time_t time_t_time;
    struct tm* tm_info;

    time(&time_t_time);
    tm_info = localtime(&time_t_time);

    strftime(buf, MAX_BUFFER_SIZE, "%Y%m%d4%H%M%S", tm_info);
}


static void getOffset(Timer *timer, char *buf)
{
    sg_sprintf(buf, "19800");
}


unsigned int getMinimumDelayPossibleInMicroSeconds(Timer *timer)
{
    return 500;
}


void minimumDelay(Timer *timer)
{
    usleep(500);
}



void init_timer(Timer *timer, void *arg)
{
    timer->getTimeIn_YYYYmmdd4HHMMSS = getTimeIn_YYYYmmdd4HHMMSS;
    timer->getOffset = getOffset;
    timer->getMinimumDelayPossibleInMicroSeconds = getMinimumDelayPossibleInMicroSeconds;
    timer->minimumDelay = minimumDelay;
}


void release_timer(Timer* timer)
{
}
