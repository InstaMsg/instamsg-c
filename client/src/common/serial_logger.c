#include "./include/serial_logger.h"

void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg)
{
    /* Register write-callback. */
	serialLoggerInterface->write = serial_logger_write;

    connect_underlying_medium_guaranteed(serialLoggerInterface);
}


void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface)
{
    release_underlying_medium_guaranteed(serialLoggerInterface);
}
