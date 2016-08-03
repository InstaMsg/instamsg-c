#include <string.h>

#include "./include/gps.h"

void trim_buffer_to_contain_only_first_GPRMC_sentence(unsigned char *buffer, int bufferLength)
{
    memset(buffer, 0, bufferLength);
}

#if GPS_TIME_SYNC_PRESENT == 1
/*
 * Fills in the time-coordinates from GPRMC-sentence, as per http://aprs.gids.nl/nmea/#rmc
 *
 * In particular, following fields need to be filled
 *
 *      dateParams->tm_year  // year    in YY
 *      dateParams->tm_mon;  // month   in MM    (01-12)
 *      dateParams->tm_mday; // day     in DD    (01-31)
 *      dateParams->tm_hour; // hour    in hh    (00-23)
 *      dateParams->tm_min;  // minute  in mm    (00-59)
 *      dateParams->tm_sec;  // second  in ss    (00-59)
 *
 *
 * Returns SUCCESS on successful fetching of all time-coordinates.
 * Else returns FAILURE.
 */
int fill_in_time_coordinates_from_GPRMC_sentence(char *buffer, DateParams *dateParams)
{
    return FAILURE;
}
#endif

