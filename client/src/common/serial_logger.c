#include "./include/serial_logger.h"

void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg)
{
    connect_underlying_medium_guaranteed(serialLoggerInterface);

    /* Register write-callback. */
	serialLoggerInterface->write = serial_logger_write;
}


void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface)
{
    release_underlying_medium_guaranteed(serialLoggerInterface);
}
