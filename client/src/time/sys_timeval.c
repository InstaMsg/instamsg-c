/*******************************************************************************
 * Contributors:
 *
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/


#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "include/time.h"


struct TimerObj {
        struct timeval end_time;
};

#define GET_IMPLEMENTATION_SPECIFIC_TIMER_OBJ(timer) ((struct TimerObj*)(timer->obj))

void getTimeIn_YYYYmmdd4HHMMSS(Timer *timer, unsigned char *buf, int maxValueLenAllowed)
{
    time_t time_t_time;
    struct tm* tm_info;

    time(&time_t_time);
    tm_info = localtime(&time_t_time);

    strftime(buf, maxValueLenAllowed, "%Y%m%d4%H%M%S", tm_info);
}


void getOffset(Timer *timer, unsigned char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "19800");
}


Timer* get_new_timer()
{
	Timer *timer = (Timer*) malloc(sizeof(Timer));
	timer->obj = malloc(sizeof(struct TimerObj));

    timer->getTimeIn_YYYYmmdd4HHMMSS = getTimeIn_YYYYmmdd4HHMMSS;
    timer->getOffset = getOffset;
}


void release_timer(Timer* timer)
{
    if(timer != NULL)
    {
        if(timer->obj != NULL)
        {
	        free(timer->obj);
        }

	    free(timer);
    }
}
