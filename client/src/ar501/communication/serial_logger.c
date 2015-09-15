/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/


#include "inc/hw_memmap.h"

#include "instamsg_vendor.h"
#include "./serial_logger.h"
#include "../uart/uart_utils.h"
#include "../../common/include/globals.h"


static int serial_logger_write(SerialLoggerInterface* serialLoggerInterface, unsigned char* buffer, int len)
{
    UARTSend(UART0_BASE, buffer, len);
    return SUCCESS;
}


void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg)
{
    /*
     * UART-initialiazation.
     */
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /*
     * Enable TX.
     */
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_1);

    /*
     * Configure UART-clocking.
     */
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    UARTEnable(UART0_BASE);




    /* Register write-callback. */
	serialLoggerInterface->write = serial_logger_write;
}


void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface)
{
    /*
     * Nothing to be done as such.
     * Multiple re-inits (without any so-called previous cleanups) SHOULD not cause any issues.
     */
}
