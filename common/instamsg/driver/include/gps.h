#ifndef INSTAMSG_TIME_COMMON
#define INSTAMSG_TIME_COMMON

void fill_in_gps_nmea_blob_until_buffer_fills_or_time_expires(unsigned char *buffer, int bufferLength, int maxTime);
void trim_buffer_to_contain_only_first_GPRMC_sentence(unsigned char *buffer, int bufferLength);

#endif
