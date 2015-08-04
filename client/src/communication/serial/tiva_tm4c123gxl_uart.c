/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
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

static char UARTRead(void)
{
    uint32_t ui32Status;
    char ch;

    /*
     * This code was picked up, where this method was called upon interrupts.
     * But here, this is being called synchronously.
     *
     * TODO: Clean this interrupt-related code.
     */
    //
    // Get the interrrupt status.
    //
    ui32Status = ROM_UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    ROM_UARTIntClear(UART0_BASE, ui32Status);

    //
    // Loop while there are characters in the receive FIFO.
    //
    if(1)
    {
        /*
         * Get the character in a blocking-manner.
         */
        ch = ROM_UARTCharGet(UART0_BASE);
    }

    return ch;
}

//*****************************************************************************
//
// Send a string to the UART.
//
//*****************************************************************************
static void UARTSend(const unsigned char *string)
{
    while(*string != 0)
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
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();

    //
    // Set the clocking to run directly from the crystal.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //
    // Enable the GPIO pins for the LED (PF2).
    //
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

    //
    // Enable the peripherals used by this example.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable processor interrupts.
    //
    ROM_IntMasterEnable();

    //
    // Set GPIO A0 and A1 as UART pins.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Configure the UART for 9600, 8-N-1 operation.
    //
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    //
    // Enable the UART interrupt.
    //
    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}


static int tiva_serial_read(Serial* serial, unsigned char* buffer, int len)
{
    int pos = 0;
    while(len-- > 0)
    {
        char ch = UARTRead();
        buffer[pos++] = ch;
    }

    return SUCCESS;
}


static int tiva_serial_write(Serial* serial, unsigned char* buffer, int len)
{
    UARTSend(buffer);
    return SUCCESS;
}


void init_serial_interface(Serial *serial, void *arg)
{
    init();

    // Register read-callback.
	serial->read = tiva_serial_read;

    // Register write-callback.
	serial->write = tiva_serial_write;
}


void release_serial_interface(Serial *serial)
{
    /*
     * Nothing to be done as such.
     * Multiple re-inits (without any so-called previous cleanups) SHOULD not cause any issues.
     */
}
