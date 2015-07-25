typedef struct Timer Timer;

struct Timer
{

    void *obj;

	void (*init_timer)(Timer*);
	char (*expired)(Timer*);
	void (*countdown_ms)(Timer*, unsigned int);
	void (*countdown)(Timer*, unsigned int);
	int (*left_ms)(Timer*);
};

Timer* get_new_timer();
void release_timer(Timer*);
