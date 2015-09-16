#ifndef INSTAMSG_TIME_COMMON
#define INSTAMSG_TIME_COMMON

#include "device_time.h"

void init_timer(Timer *timer, void *arg);
void release_timer(Timer* timer);

void connect_underlying_time_medium_guaranteed(Timer *timer);
unsigned int getMinimumDelayPossibleInMicroSeconds(Timer *timer);
void minimumDelay(Timer *timer);
void release_underlying_time_medium_guaranteed(Timer* timer);

Timer singletonUtilityTimer;

#endif


