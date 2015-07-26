#include <pthread.h>

void init_func()
{
}

void init_thread_library()
{
    static pthread_once_t random_is_initialized = PTHREAD_ONCE_INIT;
    pthread_once(&random_is_initialized, init_func);
}

void create_and_init_thread(void *start_func, void *arg)
{
    pthread_t thrId;
    pthread_create(&thrId, NULL, start_func, arg);
}

