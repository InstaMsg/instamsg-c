#include <pthread.h>
#include <unistd.h>

void create_and_init_thread(void *start_func, void *arg)
{
    pthread_t thrId;
    pthread_create(&thrId, NULL, start_func, arg);
}

void thread_sleep(int seconds)
{
    sleep(seconds);
}
