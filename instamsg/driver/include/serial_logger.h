#ifndef INSTAMSG_SERIAL_LOGGER_COMMON
#define INSTAMSG_SERIAL_LOGGER_COMMON


/*
 * Global-functions callable.
 */
void init_serial_logger();
int serial_logger_write(unsigned char* buffer, int len);
void release_serial_logger();

#endif


