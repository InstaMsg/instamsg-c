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
 * Once this is completed, the watchdog-timer starts counting down from "n" seconds to 0.
 * Then either of the following must happen ::
 *
 * a)
 * Counter reaches 0.
 *
 * In this case, the global "watchdog_expired" variable must be set to 1.
 * Also, if "immediate" is 1, the device must be reboooted immediately.
 *
 * b)
 * "watch_dog_disable()" (the global API-function) is called by the callee.
 *
 * In this case, the countdown-timer stops, and the device must not be reset/restarted.
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
