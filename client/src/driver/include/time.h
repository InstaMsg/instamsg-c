#ifndef INSTAMSG_TIME_COMMON
#define INSTAMSG_TIME_COMMON

#include "device_time.h"

/*
 * Global-functions callable.
 */
void init_timer(Timer *timer, void *arg);
void release_timer(Timer* timer);


/*
 * Must not be called directly.
 * Instead must be called as ::
 *
 *      timer->getMinimumDelayPossibleInMicroSeconds
 *      timer->minimumDelay
 */
unsigned int getMinimumDelayPossibleInMicroSeconds(Timer *timer);
void minimumDelay(Timer *timer);


/*
 * Internally Used.
 * Must not be called by anyone.
 */
void connect_underlying_time_medium_guaranteed(Timer *timer);
void release_underlying_time_medium_guaranteed(Timer* timer);

Timer singletonUtilityTimer;

#endif


