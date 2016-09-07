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




#ifndef INSTAMSG_GPS_COMMON
#define INSTAMSG_GPS_COMMON

#include "./time.h"

#define GPS             PROSTR("[GPS] ")
#define GPS_ERROR       PROSTR("[GPS_ERROR] ")

#define MAX_TIME_ALLOWED_FOR_ONE_GPS_ITERATION      20


/*
 * Common-Section
 */
void get_gps_sentence(unsigned char *buffer, int bufferLength, const char *sentenceType);
void send_gps_location(const char *gps_location_string);
int fill_in_time_coordinates_from_sentence(char *buffer, int bufferLength, DateParams *dateParams, const char *sentenceType);

/*
 * Device-Section
 */
void fill_in_gps_nmea_blob_until_buffer_fills_or_time_expires(unsigned char *buffer, int bufferLength, int maxTime);

#endif
