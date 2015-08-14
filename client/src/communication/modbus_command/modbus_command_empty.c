/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "instamsg_vendor.h"
#include "../../common/include/globals.h"


static int modbus_command_stub_read(ModbusCommandInterface* modbusCommandInterface, unsigned char* buffer, int len, unsigned char guaranteed)
{
    return SUCCESS;
}


static int modbus_command_stub_write(ModbusCommandInterface* modbusCommandInterface, unsigned char* buffer, int len)
{
    return SUCCESS;
}


void init_modbus_command_interface(ModbusCommandInterface *modbusCommandInterface, void *arg)
{
    // Register read-callback.
	modbusCommandInterface->read = modbus_command_stub_read;

    // Register write-callback.
	modbusCommandInterface->write = modbus_command_stub_write;
}


void release_modbus_command_interface(ModbusCommandInterface *modbusCommandInterface)
{
}
