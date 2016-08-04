#ifndef INSTAMSG_GPS_COMMON
#define INSTAMSG_GPS_COMMON

#include "./time.h"

#define GPS             PROSTR("[GPS] ")
#define GPS_ERROR       PROSTR("[GPS_ERROR] ")

void fill_in_gps_nmea_blob_until_buffer_fills_or_time_expires(unsigned char *buffer, int bufferLength, int maxTime);

void trim_buffer_to_contain_only_first_required_sentence_type(unsigned char *buffer, int bufferLength, const char *sentenceType);
int fill_in_time_coordinates_from_GPRMC_sentence(char *buffer, int bufferLength, DateParams *dateParams);

#endif
