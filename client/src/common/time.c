#include "./include/time.h"

void init_timer(Timer *timer, void *arg)
{
    timer->getMinimumDelayPossibleInMicroSeconds = getMinimumDelayPossibleInMicroSeconds;
    timer->minimumDelay = minimumDelay;

    connect_underlying_time_medium_guaranteed(timer);
}


void release_timer(Timer* timer)
{
    release_underlying_time_medium_guaranteed(timer);
}
