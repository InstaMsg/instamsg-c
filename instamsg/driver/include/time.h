#ifndef INSTAMSG_TIME_COMMON
#define INSTAMSG_TIME_COMMON


#define COUNTDOWN "COUNTDOWN"
volatile unsigned char countdownFinished;


/*
 * Global-functions callable.
 */
unsigned long getMinimumDelayPossibleInMicroSeconds();
void minimumDelay();

#endif


