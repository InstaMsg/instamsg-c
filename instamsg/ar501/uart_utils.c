#include <stdint.h>
#include "driverlib/rom.h"

void UARTSend(const unsigned int UART_ID, const unsigned char *buf, unsigned int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        ROM_UARTCharPut(UART_ID, (*buf));
        buf++;
    }
}

