#include "./instamsg_vendor.h"

#include "../common/include/globals.h"
#include "../common/include/instamsg.h"
#include "../common/include/time.h"

void UARTSend(const unsigned int UART_ID, const unsigned char *buf, unsigned int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        ROM_UARTCharPut(UART_ID, (*buf));
        buf++;
    }
}

