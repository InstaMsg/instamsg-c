#ifndef INSTAMSG_TIME_COMMON
#define INSTAMSG_TIME_COMMON


#define CLOCK           "[CLOCK] "
#define CLOCK_ERROR     "[CLOCK-ERROR] "

typedef struct DateParams DateParams;
struct DateParams
{
    int tm_year;
	int tm_mon;
	int tm_mday;
	int tm_wday;
	int tm_yday;
	int tm_hour;
	int tm_min;
	int tm_sec;
};

/*
 * Global-functions callable.
 */
void init_global_timer();
unsigned long getMinimumDelayPossibleInMicroSeconds();
void minimumDelay();
unsigned long getCurrentTick();

void extract_date_params(unsigned long t, DateParams *tm);
int sync_system_clock(DateParams *dateParams);

#endif


