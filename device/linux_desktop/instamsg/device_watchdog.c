/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include <pthread.h>
#include <string.h>

#include "../driver/include/globals.h"


static unsigned char watchdogActive;
static int num_seconds;
static char calling_method[MAX_BUFFER_SIZE];


static void* watchdog_func(void *arg)
{
    int i;

    for(i = num_seconds; i >= 0; i--)
    {
        if(watchdogActive == 0)
        {
            return;
        }

        startAndCountdownTimer(1, 0);
    }

    /*
     * If control reaches here.. it means that the loop has run to completion, and the
     * watchdog is still active.
     */
    sg_sprintf(LOG_GLOBAL_BUFFER, "Watchdog-timer of interval [%u] seconds expired for callee [%s]... rebooting device.",
                                  num_seconds, calling_method);
    info_log(LOG_GLOBAL_BUFFER);

    rebootDevice();
}


/*
 * This method initializes the watchdog-timer.
 */
void watchdog_init()
{
    watchdogActive = 0;
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
 * The device must then be reset/restarted.
 *
 * b)
 * "watch_dog_disable()" is called.
 *
 * In this case, the countdown-timer stops, and the device must never be reset/restarted (until the entire
 * "watch_dog_reset_and_enable" loop is repeated).
 *
 */
void watchdog_reset_and_enable(int n, char *callee, void * (*func)(void *), void *arg)
{
    watchdogActive = 1;
    num_seconds = n;

    memset(calling_method, 0, sizeof(calling_method));
    strcpy(calling_method, callee);

    {
        pthread_t tid;
        pthread_create(&tid, NULL, watchdog_func, NULL);
    }
}


/*
 * This method disables the watchdog-timer.
 */
void watchdog_disable()
{
    watchdogActive = 0;
}
