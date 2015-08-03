#ifndef INSTAMSG_VENDOR
#define INSTAMSG_VENDOR

#include "../../common.h"

struct Network
{
    int socket;
    COMMUNICATION_INTERFACE(Network)
};


#include <stdio.h>
struct FileSystem
{
    FILE *fp;
    COMMUNICATION_INTERFACE(FileSystem)
};


struct Command
{
    COMMUNICATION_INTERFACE(Command)
};


#include <pthread.h>
struct Mutex
{
    pthread_mutex_t mtx;

    void (*lock)(Mutex *mutex);
    void (*unlock)(Mutex *mutex);
};


#include <sys/time.h>
struct Timer
{
    struct timeval end_time;
    TIMER_INTERFACE
};

#endif
