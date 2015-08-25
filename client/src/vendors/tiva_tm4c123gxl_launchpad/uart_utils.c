#include <stdint.h>
#include "driverlib/rom.h"

#include "../../common/include/globals.h"


void UARTSend(const unsigned int UART_ID, const unsigned char *string, unsigned int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        //
        // Write the next character to the UART.
        //
        ROM_UARTCharPut(UART_ID, (*string));
        string++;
    }
}


int UARTRecv(const unsigned int UART_ID, unsigned char *buf, unsigned int len, unsigned int timeout)
{
    int i;
    int rc = FAILURE;

    if(timeout == NO_TIMEOUT)
    {
        for(i = 0; i < len; i++)
        {
            *buf = ROM_UARTCharGet(UART_ID);
            buf++;
        }

        rc = SUCCESS;
    }

    return rc;
}


