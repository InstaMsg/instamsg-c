#ifndef INSTAMSG_TIME
#define INSTAMSG_TIME

typedef struct Timer Timer;

struct Timer
{
    /*
     * Any extra fields may be added here.
     */

    unsigned int (*getMinimumDelayPossibleInMicroSeconds)(Timer *timer);
    void (*minimumDelay)(Timer *timer);
};

#endif
