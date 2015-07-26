void create_and_init_thread(void *start_func, void *arg);
void thread_sleep(int seconds);

#ifndef MQTT_MUTEX
#define MQTT_MUTEX
struct Mutex
{
    void *obj;

    void (*lock)(struct Mutex*);
    void (*unlock)(struct Mutex*);
};


struct Mutex* get_new_mutex();
void release_mutex(struct Mutex*);
#endif
