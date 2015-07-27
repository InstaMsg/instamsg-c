#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "include/threading.h"

void create_and_init_thread(void *start_func, void *arg)
{
    pthread_t thrId;
    pthread_create(&thrId, NULL, start_func, arg);
}

void thread_sleep(int seconds)
{
    sleep(seconds);
}



#define GET_IMPLEMENTATION_SPECIFIC_MUTEX_OBJ(mtx) ((pthread_mutex_t *)(mtx->obj))

void lock(struct Mutex *mtx)
{
    pthread_mutex_lock(GET_IMPLEMENTATION_SPECIFIC_MUTEX_OBJ(mtx));
}

void unlock(struct Mutex *mtx)
{
    pthread_mutex_unlock(GET_IMPLEMENTATION_SPECIFIC_MUTEX_OBJ(mtx));
}

struct Mutex* get_new_mutex()
{
    struct Mutex *mtx = (struct Mutex*) malloc(sizeof(struct Mutex));

    mtx->obj = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(GET_IMPLEMENTATION_SPECIFIC_MUTEX_OBJ(mtx), NULL);

    mtx->lock = lock;
    mtx->unlock = unlock;

    return mtx;
}

void release_mutex(struct Mutex* mtx)
{
    free(mtx->obj);
    free(mtx);
}

