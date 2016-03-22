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
static void* (*callback_func)(void *);
static void* callback_arg;


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

    if(callback_func != NULL)
    {
        callback_func(callback_arg);
    }
    else
    {
        rebootDevice();
    }
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
 * In this case, the global "watchdog_expired" variable must be set to 1.
 * Also, if "immediate" is 1, the device must be reboooted immediately.
 *
 * b)
 * "watch_dog_disable()" (the global API-function) is called by the callee.
 *
 * In this case, the countdown-timer stops, and the device must not be reset/restarted.
 *
 */
void watchdog_reset_and_enable(int n, char *callee, unsigned char immediate)
{
    watchdogActive = 1;
    num_seconds = n;
    callback_func = func;
    callback_arg = arg;

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
