/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include <pthread.h>
#include <string.h>

#include "../driver/include/globals.h"
#include "../../../common/instamsg/driver/include/watchdog.h"


static volatile unsigned char watchdog;
static volatile int watchdog_time;
static volatile unsigned char immediate_reboot;

static void* watchdog_func(void *arg)
{
    while(1)
    {
        if(watchdog == 1)
        {
            watchdog_time--;
            if(watchdog_time == 0)
            {
                watchdog_expired = 1;
                if(immediate_reboot == 1)
                {
                    print_rebooting_message();
                    exitApp();
                }
            }
        }

        sleep(1);
    }

    return NULL;
}


static pthread_t tid;

/*
 * This method initializes the watchdog-timer.
 */
void init_watchdog()
{
    watchdog = 0;

    {
        pthread_create(&tid, NULL, watchdog_func, NULL);
    }
}


/*
 * This method resets the watchdog-timer.
 *
 * n         : Number of seconds the watchdog should wait for.
 *
 * immediate : 0/1
 *             Denotes whether the device should be reset/rebooted immediately.
 *
 */
void do_watchdog_reset_and_enable(int n, unsigned char immediate)
{
    watchdog = 1;
    watchdog_time = n;
    immediate_reboot = immediate;
}


/*
 * This method disables the watchdog-timer.
 */
void do_watchdog_disable()
{
    watchdog = 0;
}
