#ifndef INSTAMSG_THREADING
#define INSTAMSG_THREADING

void create_and_init_thread(void *start_func, void *arg);
void thread_sleep(int seconds);

typedef struct Mutex Mutex;
struct Mutex
{
    void *obj;

    void (*lock)(Mutex*);
    void (*unlock)(Mutex*);
};


Mutex* get_new_mutex();
void release_mutex(Mutex*);

#endif
