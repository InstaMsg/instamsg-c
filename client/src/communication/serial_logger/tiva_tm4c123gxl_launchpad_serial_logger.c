/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/


#include "inc/hw_memmap.h"

#include "instamsg_vendor.h"
#include "uart_utils.h"
#include "../../common/include/globals.h"


static int tiva_serial_logger_write(SerialLoggerInterface* serialLoggerInterface, unsigned char* buffer, int len)
{
    UARTSend(UART0_BASE, buffer, len);
    return SUCCESS;
}


void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg)
{
    /*
     * UART-initialiazation, for serial-logger functionality.
     */

    /* Enable the peripherals. */
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);


    /* Set GPIO A1 as UART-Transmitter pins. */
#if 1
    GPIOPinConfigure(GPIO_PA0_U0RX);
#endif
    GPIOPinConfigure(GPIO_PA1_U0TX);

#if 1
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
#else
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_1);
#endif

    /* Configure the UART for 9600, 8-N-1 operation. */
    //ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 9600,
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    UARTEnable(UART0_BASE);



    /* Register write-callback. */
	serialLoggerInterface->write = tiva_serial_logger_write;
}


void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface)
{
    /*
     * Nothing to be done as such.
     * Multiple re-inits (without any so-called previous cleanups) SHOULD not cause any issues.
     */
}
