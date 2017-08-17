/*******************************************************************************
 *
 * Copyright (c) 2014 SenseGrow, Inc.
 *
 * SenseGrow Internet of Things (IoT) Client Frameworks
 * http://www.sensegrow.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Contributors:
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/



#include <stdio.h>

#include "./include/watchdog.h"
#include "./include/log.h"
#include "./include/misc.h"

volatile unsigned char watchdog_active;
volatile unsigned char watchdog_expired;

static char *trackString;


void print_rebooting_message()
{
    if(trackString == NULL)
    {
        trackString = (char*) "";
    }

    sg_sprintf(LOG_GLOBAL_BUFFER, "Watch-Dog-Timer is RESETTING DEVICE .... due to hang at [%s]", trackString);
    error_log(LOG_GLOBAL_BUFFER);
}


void watchdog_reset_and_enable(int n, const char *callee, unsigned char immediate)
{
    watchdog_active = 1;

    watchdog_expired = 0;
    trackString = callee;

    do_watchdog_reset_and_enable(n, immediate);
}


void watchdog_disable(void * (*func)(void *), void *arg)
{
    watchdog_active = 0;

    do_watchdog_disable();
    if(watchdog_expired == 1)
    {
        if(func != NULL)
        {
            func(arg);
        }
        else
        {
            print_rebooting_message();
            exitApp(0);
        }
    }
}


unsigned char time_fine_for_time_limit_function()
{
    return ( ((watchdog_active == 1) && (watchdog_expired == 0)) || (watchdog_active == 0) );
}
