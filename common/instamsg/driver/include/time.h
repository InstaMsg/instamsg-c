#ifndef INSTAMSG_TIME_COMMON
#define INSTAMSG_TIME_COMMON


#define CLOCK           "[CLOCK] "
#define CLOCK_ERROR     "[CLOCK-ERROR] "

typedef struct DateParams DateParams;
struct DateParams
{
    int tm_year;        /* year    in YY                */
	int tm_mon;         /* month   in MM    (01-12)     */
	int tm_mday;        /* day     in DD    (01-31)     */
	int tm_wday;
	int tm_yday;
	int tm_hour;        /* hour    in hh    (00-23)     */
	int tm_min;         /* minute  in mm    (00-59)     */
	int tm_sec;         /* second  in ss    (00-59)     */
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
