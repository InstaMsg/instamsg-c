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


#include "../driver/include/socket.h"
#include "../driver/include/time.h"

#include <time.h>
#include <unistd.h>
#include <errno.h>



/*
 * This method does the global-level-initialization for time (if any).
 */
void init_global_timer()
{
}


/*
 * This method returns the minimum-delay achievable via this device.
 */
unsigned long getMinimumDelayPossibleInMicroSeconds()
{
    return 500;
}


/*
 * This method ACTUALLY causes the current-device to go to sleep for the minimum-delay possible.
 */
void minimumDelay()
{
    usleep(500);
}


/*
 * This method returns the current-tick/timestamp.
 */
unsigned long getCurrentTick()
{
    return time(NULL);
}


#if GSM_TIME_SYNC_PRESENT == 1
/*
 * Returns the current-timestamp, the original of which was returned via GSM. *
 * Returns 0 in case no informaton is received from GSM (yet).
 */
unsigned long get_GSM_timestamp()
{
    return 0;
}
#endif


/*
 * Syncs the system-clock.
 *
 * Returns SUCCESS on successful-syncing.
 * Else returns FAILURE.
 */
int sync_system_clock(DateParams *dateParams, unsigned long seconds)
{
    struct timeval now;
    int rc;
    int errBackup;
    char *error_str;

    now.tv_sec = seconds;
    now.tv_usec = 0;

    rc = settimeofday(&now, NULL);
    errBackup = errno;
    if(rc == 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "%sTime-synced successfully at system.", CLOCK);
        info_log(LOG_GLOBAL_BUFFER);
    }
    else
    {
        if(errBackup == EFAULT)
        {
            error_str = "EFAULT";
        }
        else if(errBackup == EINVAL)
        {
            error_str = "EINVAL";
        }
        else if(errBackup == EPERM)
        {
            error_str = "EPERM";
        }

        sg_sprintf(LOG_GLOBAL_BUFFER, "%sTime-sync FAILED at system with errno-code [%s].", CLOCK_ERROR, error_str);
        error_log(LOG_GLOBAL_BUFFER);

        resetDevice();
    }

    return SUCCESS;
}


/*
 * This method gets the time in the following-format
 *
 *          YYYYMMDD4HHMMSS
 *          201507304155546
 */
void getTimeInDesiredFormat(char *buffer, int maxBufferLength)
{
    time_t rawtime;
    struct tm *info;

    time(&rawtime);
    info = localtime(&rawtime);

    strftime(buffer, maxBufferLength,"%Y%m%d4%H%M%S", info);
}


/*
 * This method gets the timezone-offset for this device.
 */
void getTimezoneOffset(char *buffer, int maxBufferLength)
{
#if 1
    time_t rawtime;
    struct tm *info;

    time(&rawtime);
    info = localtime(&rawtime);

    sg_sprintf(buffer, "%ld", info->tm_gmtoff);
#endif
}
