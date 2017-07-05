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



#ifndef INSTAMSG_TIME_COMMON
#define INSTAMSG_TIME_COMMON


#define CLOCK           "[CLOCK] "
#define CLOCK_ERROR     "[CLOCK-ERROR] "

typedef struct DateParams DateParams;
struct DateParams
{
    int tm_year;        /* year    in YY                */
	int tm_mon;         /* month   in MM    (01-12)     */
	int tm_mday;        /* day     in DD    (01-31)     */
	int tm_wday;
	int tm_yday;
	int tm_hour;        /* hour    in hh    (00-23)     */
	int tm_min;         /* minute  in mm    (00-59)     */
	int tm_sec;         /* second  in ss    (00-59)     */
};

/*
 * Global-functions callable.
 */
void init_global_timer();
unsigned long getMinimumDelayPossibleInMicroSeconds();
void minimumDelay();
unsigned long getCurrentTick();

#if GSM_TIME_SYNC_PRESENT == 1
unsigned long get_GSM_timestamp();
#endif

void extract_date_params(unsigned long t, DateParams *tm, const char *mode);
int sync_system_clock(DateParams *dateParams, unsigned long seconds);
void print_date_info(DateParams *tm, const char *mode);

/* This method must be used sparingly only, definitely not every second */
unsigned long getUTCTimeStamp();

void getTimeInDesiredFormat(char *buffer, int maxBufferLength);
void getTimezoneOffset(char *buffer, int maxBufferLength);

#endif
