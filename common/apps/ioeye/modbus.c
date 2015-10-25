#include "./include/modbus.h"
#include "./include/hex.h"

#include "../../instamsg/driver/include/log.h"
#include "../../instamsg/driver/include/globals.h"

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


/*
 * Function-Codes being handled ::
 *
 *  FC 1:   11 01 0013 0025 0E84
 *  FC 2:   11 02 00C4 0016 BAA9
 *  FC 3:   11 02 00C4 0016 BAA9
 *  FC 4:   11 04 0008 0001 B298
 */
static int validationCheck(char *commandNibbles)
{
    int i;
    int rc = FAILURE;

    unsigned char functionCodes[] = {
                                        '1',
                                        '2',
                                        '3',
                                        '4'
                                    };

    int commandNibblesLength = strlen(commandNibbles);
    if(commandNibblesLength < 12)
    {
        error_log(MODBUS_ERROR "Modbus-Command Length less than 12");
        return FAILURE;
    }

    /*
     * If the second byte (3rd and 4th nibbles) is not equal to 03 (reading-analog-registets), return -1
     */
    if((commandNibbles[2] != '0'))
    {
        goto exit;
    }

    for(i = 0; i < sizeof(functionCodes); i++)
    {
        if(commandNibbles[3] == functionCodes[i])
        {
            rc = SUCCESS;
            break;
        }
    }

exit:
    if(rc == FAILURE)
    {
        error_log(MODBUS_ERROR "Modbus-Command-Code [%c%c] not one of 01 02 03 04 in command [%s]",
                               commandNibbles[2], commandNibbles[3], commandNibbles);
    }

    return rc;
}


unsigned long getExpectedModbusResponseLength(char *commandNibbles)
{
    int rc;
    unsigned long i = 0;

    rc = validationCheck(commandNibbles);
    if(rc != SUCCESS)
    {
        return rc;
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


void fillPrefixIndices(char *commandNibbles, int *prefixStartIndex, int *prefixEndIndex)
{
    int rc = validationCheck(commandNibbles);
    if(rc != SUCCESS)
    {
        *prefixStartIndex = *prefixEndIndex = -1;
    }

    *prefixStartIndex = 4;
    *prefixEndIndex = 7;
}
