/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/


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
