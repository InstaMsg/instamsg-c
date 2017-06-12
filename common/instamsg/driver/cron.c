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

#include "./include/cron.h"
#include "./include/time.h"
#include "./include/sg_stdlib.h"
#include "./include/log.h"

#include <string.h>


#if CRON_ENABLED == 1

#define DUMMY_VALUE         100
static struct CronParams cronParams;

static void assignUnsignedCharValue(char *src, unsigned char *dest, int offset, int numBytes)
{
    char small[10] = {0};

    memset(small, 0, sizeof(small));
    memcpy(small, src + offset, numBytes);

    *dest = sg_atoi(small);
}


static int processParsedCronField(char *cron, char *field, char *field_desc, unsigned char *cronParam)
{
    if(1)
    {
        if(field == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s%s not present in cron in [%s]"), CRON_ERROR, field_desc, cron);
            error_log(LOG_GLOBAL_BUFFER);

            return FAILURE;
        }
        else
        {
            if(strcmp(field, "*") == 0)
            {
                *cronParam = 0;
            }
            else
            {
                *cronParam = sg_atoi(field);
            }
        }
    }
}


static char temporaryCron[200];
int isOkToFireCronTask(char *cron)
{
    cronParams.cron_month       = DUMMY_VALUE;
    cronParams.cron_month_day   = DUMMY_VALUE;
    cronParams.cron_hour        = DUMMY_VALUE;
    cronParams.cron_minute      = DUMMY_VALUE;
    cronParams.cron_offset      = DUMMY_VALUE;

    cronParams.system_month     = DUMMY_VALUE;
    cronParams.system_month_day = DUMMY_VALUE;
    cronParams.system_hour      = DUMMY_VALUE;
    cronParams.system_minute    = DUMMY_VALUE;
    cronParams.system_offset    = DUMMY_VALUE;

    /*
     * cron will be of the format ::
     *
     *      month:month-day:hour:minute:offset:param1:param2:param3: and so on ...
     *
     * eg.
     *
     *      *:*:18:*:0:dio1:1
     *      *:*:18:*:19800:dio1:1
     *
     *      *:*:5:*:0:dio1:0
     *      *:*:5:*:19800:dio1:0
     */
    {
        char *field = NULL;

        memset(temporaryCron, 0, sizeof(temporaryCron));
        strcpy(temporaryCron, cron);

        field = strtok(temporaryCron, ":");
        if(processParsedCronField(cron, field, PROSTR("Month"), &(cronParams.cron_month)) == FAILURE)
        {
            return FAILURE;
        }

        field = strtok(NULL, ":");
        if(processParsedCronField(cron, field, PROSTR("Month-Day"), &(cronParams.cron_month_day)) == FAILURE)
        {
            return FAILURE;
        }

        field = strtok(NULL, ":");
        if(processParsedCronField(cron, field, PROSTR("Hour"), &(cronParams.cron_hour)) == FAILURE)
        {
            return FAILURE;
        }

        field = strtok(NULL, ":");
        if(processParsedCronField(cron, field, PROSTR("Minute"), &(cronParams.cron_minute)) == FAILURE)
        {
            return FAILURE;
        }

        field = strtok(NULL, ":");
        if(field == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sOffset not present in cron in [%s]"), CRON_ERROR, cron);
            error_log(LOG_GLOBAL_BUFFER);

            return FAILURE;
        }
        else
        {
            if(strcmp(field, "*") == 0)
            {
                cronParams.cron_offset = 0;
            }
            else
            {
                cronParams.cron_offset = sg_atoi(field);
            }
        }
    }

    {
        char small[20] = {0};

        /*
         * The system-time is in the format ::
         *
         *      YYYYMMDD4HHMMSS
         *      201507304155546
         */
        getTimeInDesiredFormat(small, sizeof(small));

        assignUnsignedCharValue(small, &(cronParams.system_month), 4, 2);
        assignUnsignedCharValue(small, &(cronParams.system_month_day), 6, 2);
        assignUnsignedCharValue(small, &(cronParams.system_hour), 9, 2);
        assignUnsignedCharValue(small, &(cronParams.system_minute), 11, 2);

        memset(small, 0, sizeof(small));
        getTimezoneOffset(small, sizeof(small));

        cronParams.system_offset = sg_atoi(small);
    }


#if 0
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCron-Params for [%s] (month, day, hour, minute, offset) ==> [%u], [%u], [%u], [%u], [%u]"),
                                        CRON,
                                        cron,
                                        cronParams.cron_month,
                                        cronParams.cron_month_day,
                                        cronParams.cron_hour,
                                        cronParams.cron_minute,
                                        cronParams.cron_offset);
        info_log(LOG_GLOBAL_BUFFER);

        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sSystem-Time-Params (month, day, hour, minute, offset) ==> [%u], [%u], [%u], [%u], [%u]"),
                                        CRON,
                                        cronParams.system_month,
                                        cronParams.system_month_day,
                                        cronParams.system_hour,
                                        cronParams.system_minute,
                                        cronParams.system_offset);
        info_log(LOG_GLOBAL_BUFFER);
#endif


    if(cronParams.system_offset != cronParams.cron_offset)
    {
        return FAILURE;
    }

    if(cronParams.cron_month != 0)
    {
        if(cronParams.system_month == cronParams.cron_month)
        {
        }
        else
        {
            return FAILURE;
        }
    }

    if(cronParams.cron_month_day != 0)
    {
        if(cronParams.system_month_day == cronParams.cron_month_day)
        {
        }
        else
        {
            return FAILURE;
        }
    }

    if(cronParams.cron_hour != 0)
    {
        if(cronParams.system_hour == cronParams.cron_hour)
        {
        }
        else
        {
            return FAILURE;
        }
    }

    if(cronParams.cron_minute != 0)
    {
        if(cronParams.system_minute == cronParams.cron_minute)
        {
        }
        else
        {
            return FAILURE;
        }
    }

    return SUCCESS;
}


void getCronTaskFromCron(char *cron, char *cronTaskBuffer, int bufferSize)
{
    /*
     * "cron" is of format ::
     *
     *  *:*:18:*:19800:dio1:1
     */
    int i = 0, j = 0;
    int maxLength = strlen(cron);
    for(i = 0; (i < 5) && (j < maxLength); i++)
    {
        cron++;
        j++;

        while((*cron) != ':')
        {
            cron++;
            j++;
        }
    }

    if((i == 5) && (j < maxLength))
    {
        strcpy(cronTaskBuffer, cron + 1);
    }
    else
    {
        memset(cronTaskBuffer, 0, bufferSize);
    }
}

#endif
