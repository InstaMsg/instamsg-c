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



#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "../driver/include/globals.h"

#include "../../../common/instamsg/driver/include/watchdog.h"
#include "../../../common/instamsg/driver/include/misc.h"


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
