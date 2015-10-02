#include "./include/modbus.h"

void init_modbus(Modbus *modbus, void *arg);
{
    /* Register the callback */
	network->send_command_and_read_response_sync = send_command_and_read_response_sync;

    connect_underlying_modbus_medium_guaranteed(modbus);
}


void release_modbus(Modbus *modbus)
{
    release_underlying_modbus_medium_guaranteed(modbus);
}
