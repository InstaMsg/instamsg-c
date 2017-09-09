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



#include "./include/globals.h"
#include "./include/time.h"
#include "./include/log.h"
#include "./include/sg_stdlib.h"

#include <string.h>


/* 2000-03-01 (mod 400 year, immediately after feb29 */
#define LEAPOCH (946684800 + (86400 * (31 + 29)) )

#define DAYS_PER_400Y ((365 * 400) + 97)
#define DAYS_PER_100Y ((365 * 100) + 24)
#define DAYS_PER_4Y   ((365 * 4  ) + 1)

void print_date_info(DateParams *tm, const char *mode)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFrom [%s], Extracted Date in GMT [YY-MM-DD, hh:mm:ss] = [%u-%u-%u, %u:%u:%u]"), CLOCK, mode,
                                  tm->tm_year, tm->tm_mon, tm->tm_mday,
                                  tm->tm_hour, tm->tm_min, tm->tm_sec);
    info_log(LOG_GLOBAL_BUFFER);
}


void extract_date_params(unsigned long t, DateParams *tm, const char *mode)
{
	unsigned long days, secs;
	int remdays, remsecs, remyears;
	int qc_cycles, c_cycles, q_cycles;
	int years, months;
	int wday, yday, leap;
	static const char days_in_month[] = {31,30,31,30,31,31,30,31,30,31,31,29};

#if 0
	/* Reject time_t values whose year would overflow int */
	if (t < INT_MIN * 31622400LL || t > INT_MAX * 31622400LL)
    {
        return FAILURE;
    }
#endif

	secs = t - LEAPOCH;
	days = secs / 86400;
	remsecs = secs % 86400;
	if(remsecs < 0)
    {
		remsecs += 86400;
		days--;
	}

	wday = (3 + days) % 7;
	if(wday < 0)
    {
        wday += 7;
    }

	qc_cycles = days / DAYS_PER_400Y;
	remdays = days % DAYS_PER_400Y;
	if(remdays < 0)
    {
		remdays += DAYS_PER_400Y;
		qc_cycles--;
	}

	c_cycles = remdays / DAYS_PER_100Y;
	if(c_cycles == 4)
    {
        c_cycles--;
    }
	remdays -= c_cycles * DAYS_PER_100Y;

	q_cycles = remdays / DAYS_PER_4Y;
	if(q_cycles == 25)
    {
        q_cycles--;
    }
	remdays -= q_cycles * DAYS_PER_4Y;

	remyears = remdays / 365;
	if(remyears == 4)
    {
        remyears--;
    }
	remdays -= remyears * 365;

	leap = !remyears && (q_cycles || !c_cycles);
	yday = remdays + 31 + 28 + leap;
	if(yday >= 365+leap)
    {
        yday -= 365 + leap;
    }

	years = remyears + (4 * q_cycles) + (100 * c_cycles) + (400 * qc_cycles);

	for(months = 0; days_in_month[months] <= remdays; months++)
    {
		remdays -= days_in_month[months];
    }

#if 0
	if (years + 100 > INT_MAX || years + 100 < INT_MIN)
    {
		return FAILURE;
    }
#endif

	tm->tm_year = years;
	tm->tm_mon = months + 3;
	if(tm->tm_mon > 12)
    {
		tm->tm_mon -=12;
		tm->tm_year++;
	}
	tm->tm_mday = remdays + 1;
	tm->tm_wday = wday;
	tm->tm_yday = yday;

	tm->tm_hour = remsecs / 3600;
	tm->tm_min = remsecs / 60 % 60;
	tm->tm_sec = remsecs % 60;

    if(mode != NULL)
    {
        print_date_info(tm, mode);
    }
}


static char systemTime[20];
static char systemOffset[10];
static unsigned short days[4][12] =
{
    {0,    31,   60,   91,   121,  152,  182,  213,  244,  274,  305,  335},
    {366,  397,  425,  456,  486,  517,  547,  578,  609,  639,  670,  700},
    {731,  762,  790,  821,  851,  882,  912,  943,  974,  1004, 1035, 1065},
    {1096, 1127, 1155, 1186, 1216, 1247, 1277, 1308, 1339, 1369, 1400, 1430}
};


unsigned long getUTCTimeStamp()
{
    unsigned long timestamp = 946684800;        /* 1 Jan, 2000, 12:00 AM */
    int offset = 0;

    unsigned short temp1 = 0;
    unsigned short temp2 = 0;

    /*
     * The system-time is in the format ::
     *
     *      YYYYMMDD4HHMMSS
     *      201507304155546
     */
    memset(systemTime, 0, sizeof(systemTime));
    getTimeInDesiredFormat(systemTime, sizeof(systemTime));

    assignUnsignedShortValue(systemTime, &temp1, 0, 4);                                 /* year from 2000-2099 */
    temp1 = temp1 - 2000;
    assignUnsignedShortValue(systemTime, &temp2, 4, 2);                                 /* month from 1-12 */
    temp2 = temp2 - 1;
    timestamp = timestamp + ( ((temp1 / 4) * ((365 * 4) + 1)) * (24 * 60 * 60) );
    timestamp = timestamp + ( (days[temp1 % 4][temp2]) * (24 * 60 * 60) );

    assignUnsignedShortValue(systemTime, &temp1, 6, 2);                                 /* day from 1-31 */
    temp1 = temp1 - 1;
    timestamp = timestamp + (temp1 * (24 * 60 * 60));

    assignUnsignedShortValue(systemTime, &temp1, 9, 2);                                 /* hour from 0-23 */
    timestamp = timestamp + (temp1 * (60 * 60));

    assignUnsignedShortValue(systemTime, &temp1, 11, 2);                                /* minute from 0-59 */
    timestamp = timestamp + (temp1 * 60);

    assignUnsignedShortValue(systemTime, &temp1, 13, 2);                                /* second from 0-59 */
    timestamp = timestamp + temp1;

    memset(systemOffset, 0, sizeof(systemOffset));
    getTimezoneOffset(systemOffset, sizeof(systemOffset));

    offset = sg_atoi(systemOffset);
    if(offset >= 0)
    {
        timestamp = timestamp - offset;
    }
    else
    {
        offset = offset * (-1);
        timestamp = timestamp + offset;
    }

    return timestamp;
}


static DateParams dateParams;
void addTwoDigitTimeField(char *buffer, int field, const char *follower)
{
    char smallBuffer[4] = {0};

    if(field < 10)
    {
        sg_sprintf(smallBuffer, "0%u%s", field, follower);
    }
    else
    {
        sg_sprintf(smallBuffer, "%u%s", field, follower);
    }

    strcat(buffer, smallBuffer);
}


/*
 * This method gets the system-time as UTC in the following-format
 *
 *          YYYYMMDD4HHMMSS
 *          201507304155546
 */
void getUTCTimeInDesiredFormat(char *buffer, int maxBufferLength)
{
    unsigned long utcTimestamp = getUTCTimeStamp();
    extract_date_params(utcTimestamp, &dateParams, NULL);

    strcat(buffer, "20");
    addTwoDigitTimeField(buffer, dateParams.tm_year, "");
    addTwoDigitTimeField(buffer, dateParams.tm_mon, "");
    addTwoDigitTimeField(buffer, dateParams.tm_mday, "");
    strcat(buffer, "4");
    addTwoDigitTimeField(buffer, dateParams.tm_hour, "");
    addTwoDigitTimeField(buffer, dateParams.tm_min, "");
    addTwoDigitTimeField(buffer, dateParams.tm_sec, "");
}
