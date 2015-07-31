#ifndef INSTAMSG_TIME
#define INSTAMSG_TIME

typedef struct Timer Timer;
struct Timer
{

    void *obj;

};

Timer* get_new_timer();
void release_timer(Timer*);

#endif
