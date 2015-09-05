#include <stdint.h>
#include "driverlib/rom.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"


#include "../../common/include/globals.h"
#include "../../common/include/instamsg.h"

#if 1
void UARTSend(const unsigned int UART_ID, unsigned char *buf, unsigned int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        //
        // Write the next character to the UART.
        //
        ROM_UARTCharPut(UART_ID, (*buf));
        //ROM_UARTCharPut(UART2_BASE, (*buf));
        //ROM_UARTCharPut(UART_ID, 'a');
        buf++;
    }

#if 0
    ROM_UARTDisable(UART_ID);
    ROM_UARTEnable(UART_ID);
#endif
}


int UARTRecv(const unsigned int UART_ID, unsigned char *buf, unsigned int len, unsigned int timeout)
{
    int i;
    char ch;
    unsigned long j, numDelays;

    int rc = FAILURE;

    
    //info_log("debug1 UARTRecv");
    if(timeout == NO_TIMEOUT)
    {
#if 0
        while(1)
        {
        while(ROM_UARTCharsAvail(UART_ID))
        {
            *buf = ROM_UARTCharGetNonBlocking(UART_ID);
                info_log("jeez [%d]", *buf);
        }
        }
#endif
    //info_log("debug2 UARTRecv");
        for(i = 0; i < len; i++)
        {
    info_log("debug3 UARTRecv");
            *buf = ROM_UARTCharGet(UART_ID);
            //ROM_UARTCharPut(UART_ID, *buf);
        //ROM_UARTCharPut(UART0_BASE, *buf);
        //ROM_UARTCharPut(UART0_BASE, (*buf));
            
                //ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);
                info_log("[%c]", *buf);
        //ROM_UARTCharPut(UART1_BASE, *buf);
            buf++;
        }

        rc = SUCCESS;
    }
    else
    {
        numDelays = (1000000 / singletonUtilityTimer.getMinimumDelayPossibleInMicroSeconds(&singletonUtilityTimer)) *
                    timeout;
        j = 0;

        // We wait only for the first character to arrive.
        // If the first character arrives, then it is guaranteed that rest will follow too.
        // If the timeout has to be happen, it will happen at the first character itself.
        do
        {
            ch = ROM_UARTCharGetNonBlocking(UART_ID);
            j++;

        } while((ch == -1) && (j < numDelays));

        // If we still did not receive, we have timed-out.
        if(ch == -1)
        {
            info_log("Timeout occurred while waiting to read from UART [%u]", UART_ID);
            rc =  SOCKET_READ_TIMEOUT;
        }
        else
        {
            *buf = ch;
            info_log("%c", *buf);

            // Read the remaining characters.
            for(i = 0; i < (len - 1); i++)
            {
                *buf = ROM_UARTCharGet(UART_ID);
                info_log("%c", *buf);
                buf++;
            }

            rc = SUCCESS;
        }

    }

    return rc;
}
#endif

