#ifndef INSTAMSG_SERIAL_LOGGER_COMMON
#define INSTAMSG_SERIAL_LOGGER_COMMON

#include "device_serial_logger.h"

void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg);
void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface);

void connect_underlying_serial_logger_medium_guaranteed(SerialLoggerInterface *serialLoggerInterface);
int serial_logger_write(SerialLoggerInterface* serialLoggerInterface, unsigned char* buffer, int len);
void release_underlying_serial_logger_medium_guaranteed(SerialLoggerInterface *serialLoggerInterface);

#endif


