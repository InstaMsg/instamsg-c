#include <stdint.h>
#include "driverlib/rom.h"


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


