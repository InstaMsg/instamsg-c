#ifndef WATCHDOG_COMMON_IMPL
#define WATCHDOG_COMMON_IMPL

#include <stdio.h>

#include "./include/watchdog.h"
#include "./include/log.h"
#include "./include/misc.h"


static char *trackString;


void print_rebooting_message()
{
    if(trackString == NULL)
    {
        trackString = "";
    }

    sg_sprintf(LOG_GLOBAL_BUFFER, "Watch-Dog-Timer is RESETTING DEVICE .... due to hang at [%s]", trackString);
    error_log(LOG_GLOBAL_BUFFER);
}


void watchdog_reset_and_enable(int n, char *callee, unsigned char immediate)
{
    watchdog_expired = 0;
    trackString = callee;

    do_watchdog_reset_and_enable(n, immediate);
}


void watchdog_disable(void * (*func)(void *), void *arg)
{
    do_watchdog_disable();

    if(watchdog_expired == 1)
    {
        print_rebooting_message();

        if(func != NULL)
        {
            func(arg);
        }
        else
        {
            rebootDevice();
        }
    }
}
#endif
