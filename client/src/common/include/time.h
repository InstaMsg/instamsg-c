#ifndef INSTAMSG_TIME_COMMON
#define INSTAMSG_TIME_COMMON

#include "time/time.h"

void init_timer(Timer *timer, void *arg);
void release_timer(Timer* timer);

unsigned int getMinimumDelayPossibleInMicroSeconds(Timer *timer);
void minimumDelay(Timer *timer);

#endif


