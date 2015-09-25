#ifndef INSTAMSG_SERIAL_LOGGER_COMMON
#define INSTAMSG_SERIAL_LOGGER_COMMON

#include "device_serial_logger.h"

/*
 * Global-functions callable.
 */
void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg);
void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface);


/*
 * Must not be called directly.
 * Instead must be called as ::
 *
 *      serialLoggerInterface->write
 */
int serial_logger_write(SerialLoggerInterface* serialLoggerInterface, unsigned char* buffer, int len);


/*
 * Internally Used.
 * Must not be called by anyone.
 */
void connect_underlying_serial_logger_medium_guaranteed(SerialLoggerInterface *serialLoggerInterface);
void release_underlying_serial_logger_medium_guaranteed(SerialLoggerInterface *serialLoggerInterface);

#endif


