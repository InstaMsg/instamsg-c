#ifndef INSTAMSG_TIME
#define INSTAMSG_TIME

#include <sys/time.h>

typedef struct Timer Timer;

struct Timer
{
    /*
     * Any extra fields may be added here.
     */
    struct timeval end_time;

    unsigned int (*getMinimumDelayPossibleInMicroSeconds)(Timer *timer);
    void (*minimumDelay)(Timer *timer);
};

#endif
