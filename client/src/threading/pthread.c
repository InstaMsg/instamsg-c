#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "instamsg_vendor.h"

void create_and_init_thread(void *start_func, void *arg)
{
    pthread_t thrId;
    pthread_create(&thrId, NULL, start_func, arg);
}


void thread_sleep(int seconds)
{
    sleep(seconds);
}



static void lock(struct Mutex *mutex)
{
    pthread_mutex_lock(&(mutex->mtx));
}


static void unlock(struct Mutex *mutex)
{
    pthread_mutex_unlock(&(mutex->mtx));
}


void init_mutex(struct Mutex *mutex)
{
    pthread_mutex_init(&(mutex->mtx), NULL);

    mutex->lock = lock;
    mutex->unlock = unlock;
}


void release_mutex(struct Mutex* mtx)
{
}

