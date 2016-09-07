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



#include "../driver/include/gps.h"
#include "../driver/include/log.h"

/*
 * This method fills the "buffer" with NMEA-blob.
 *
 * This method must return whenever any of the following holds true ::
 *
 *          * Either the whole of "buffer" fills upto "bufferLength", OR
 *          * "maxTime" has elapsed while waiting/filling-in NMEA-blob.
 */
void fill_in_gps_nmea_blob_until_buffer_fills_or_time_expires(unsigned char *buffer, int bufferLength, int maxTime)
{
}


#if SEND_GPS_LOCATION == 1
/*
 * The device may decide how to send the gps-location-string to the server.
 * Or how to consume this string in general.
 */
void send_gps_location(const char *gps_location_string)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, "GPS-Location String ==> [%s]", gps_location_string);
    info_log(LOG_GLOBAL_BUFFER);
}
#endif

