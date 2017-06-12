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



#ifndef INSTAMSG_CRON_COMMON
#define INSTAMSG_CRON_COMMON

#include "device_defines.h"

#if CRON_ENABLED == 1

#define CRON           "[CRON] "
#define CRON_ERROR     "[CRON-ERROR] "

struct CronParams
{
    unsigned char cron_month;               /* 1-12 */
    unsigned char cron_month_day;           /* 1-31 */
    unsigned char cron_hour;                /* 0-23 */
    unsigned char cron_minute;              /* 0-59 */
    int cron_offset;

    unsigned char system_month;             /* 1-12 */
    unsigned char system_month_day;         /* 1-31 */
    unsigned char system_hour;              /* 0-23 */
    unsigned char system_minute;            /* 0-59 */
    int system_offset;
};


int isOkToFireCronTask(char *cron);
void getCronTaskFromCron(char *cron, char *cronTaskBuffer, int bufferSize);
void runCronTask(char *task);
#endif

#endif
