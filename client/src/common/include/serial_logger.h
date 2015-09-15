#ifndef SERIAL_LOGGER_COMMON
#define SERIAL_LOGGER_COMMON

#include "communication/serial_logger.h"

void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg);
void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface);

#endif


