/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "instamsg_vendor.h"
#include "../../common/include/globals.h"



//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


//*****************************************************************************
//
// Send a string to the UART.
//
//*****************************************************************************
static void UARTSend(const unsigned char *string, unsigned int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        //
        // Write the next character to the UART.
        //
        ROM_UARTCharPut(UART0_BASE, (*string));
        string++;
    }
}


static void init(void)
{
    //
    // Enable the peripherals used by this example.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);


    //
    // Set GPIO A1 as UART-Transmitter pins.
    //
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_1);

    //
    // Configure the UART for 9600, 8-N-1 operation.
    //
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

}


static int tiva_serial_logger_write(SerialLoggerInterface* serialLoggerInterface, unsigned char* buffer, int len)
{
    UARTSend(buffer, len);
    return SUCCESS;
}


void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg)
{
    init();

    // Register write-callback.
	serialLoggerInterface->write = tiva_serial_logger_write;
}


void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface)
{
    /*
     * Nothing to be done as such.
     * Multiple re-inits (without any so-called previous cleanups) SHOULD not cause any issues.
     */
}
