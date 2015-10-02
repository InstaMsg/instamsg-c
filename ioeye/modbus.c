#include "./include/modbus.h"
#include "./include/hex.h"

#include "../instamsg/driver/include/log.h"

#include <string.h>


#define MODBUS_ERROR "[MODBUS_ERROR] "

void init_modbus(Modbus *modbus, void *arg)
{
    /* Register the callback */
	modbus->send_command_and_read_response_sync = modbus_send_command_and_read_response_sync;

    connect_underlying_modbus_medium_guaranteed(modbus);
}


void release_modbus(Modbus *modbus)
{
    release_underlying_modbus_medium_guaranteed(modbus);
}


unsigned long getExpectedModbusResponseLength(char *commandNibbles)
{
    unsigned long i = 0;
    int commandNibblesLength = strlen(commandNibbles);

    if(commandNibblesLength < 12)
    {
        error_log(MODBUS_ERROR "Modbus-Command Length less than 12");
        return -1;
    }

    /*
     * If the second byte (3rd and 4th nibbles) is not equal to 03 (reading-analog-registets), return -1
     */
    if((commandNibbles[2] != '0') || (commandNibbles[3] != '3'))
    {
        error_log(MODBUS_ERROR "Modbus-Command Not for reading analog-registers [%c] [%c]", commandNibbles[2], commandNibbles[3]);
        return -1;
    }

    /*
     * The 9, 10, 11, 12 nibbles contain the number of registers to be read.
     */
    i = i + (16 * 16 * 16 * getIntValueOfHexChar(commandNibbles[8]));
    i = i + (16 * 16 *      getIntValueOfHexChar(commandNibbles[9]));
    i = i + (16 *           getIntValueOfHexChar(commandNibbles[10]));
    i = i + (               getIntValueOfHexChar(commandNibbles[11]));

    i = i * 2;      /* 2 bytes per register */

    i = i + 3;      /* Id, Code, Bytes-Counter-Byte in the beginning*/
    i = i + 2;      /* 2 bytes for CRC in the end */

    return i;
}
