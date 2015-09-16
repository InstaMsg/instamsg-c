#include "./include/time.h"

void init_timer(Timer *timer, void *arg)
{
    connect_underlying_time_medium_guaranteed(timer);

    timer->getMinimumDelayPossibleInMicroSeconds = getMinimumDelayPossibleInMicroSeconds;
    timer->minimumDelay = minimumDelay;
}


void release_timer(Timer* timer)
{
    release_underlying_time_medium_guaranteed(timer);
}
