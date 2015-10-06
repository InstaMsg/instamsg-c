#ifndef INSTAMSG_TIME_COMMON
#define INSTAMSG_TIME_COMMON


/*
 * Global-functions callable.
 */
void init_global_timer();
unsigned long getMinimumDelayPossibleInMicroSeconds();
void minimumDelay();
unsigned long getCurrentTick();

#endif


