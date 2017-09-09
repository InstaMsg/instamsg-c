/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "device_defines.h"


#include "../driver/include/gps.h"

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
}
#endif

