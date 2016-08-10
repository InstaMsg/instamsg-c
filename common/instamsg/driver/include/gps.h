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
