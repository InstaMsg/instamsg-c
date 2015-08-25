/*******************************************************************************
 * Contributors:
 *
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"



#include "instamsg_vendor.h"


/* This method fills in the string-value of device-manufacturer in "buf".
 *
 * Note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation
 * does not need to bother about that.
 *
 * Also, the device-implementation must take care that the length of manufacturer-value (including null-terminator)
 * is not greater than "maxValueLenAllowed".
 */
static void getManufacturer(System *system, char *buf, int maxValueLenAllowed)
{
}


/* This method fills in the string-value of device-serial-number in "buf".
 *
 * Note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation
 * does not need to bother about that.
 *
 * Also, the device-implementation must take care that the length of serial-number-value (including null-terminator)
 * is not greater than "maxValueLenAllowed".
 */
static void getSerialNumber(System *system, char *buf, int maxValueLenAllowed)
{
}


/*
 * This method reboots the device, thus completely re-initializing the system.
 */
static void rebootDevice(System *sys)
{
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void init_system_utils(System *system, void *arg)
{
    system->getManufacturer = getManufacturer;
    system->getSerialNumber = getSerialNumber;
    system->rebootDevice = rebootDevice;
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void release_system_utils(System *system)
{
}


/*
 * This method does the one-time system-initialization.
 * This code is called right at the beginning of InstaMsg-binary.
 */
void SYSTEM_GLOBAL_INIT()
{
    /*
     * Set the clocking to run directly from the crystal.
     */
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);



    /*
     * UART-initialiazation, for serial-logger functionality.
     */

    // Enable the peripherals.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);


    // Set GPIO A1 as UART-Transmitter pins.
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_1);

    // Configure the UART for 9600, 8-N-1 operation.
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));


    // Enable the peripherals.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);


    // Set GPIO A1 as UART-Transmitter pins.
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure GPIO pins for UART1 HW flow control
    GPIOPinConfigure(GPIO_PC4_U1RTS);
    GPIOPinConfigure(GPIO_PC5_U1CTS);
    ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    // Configure the UART for 9600, 8-N-1 operation.
    ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    // Enable RTS/CTS HW flow control for UART1
    UARTFlowControlSet(UART1_BASE, UART_FLOWCONTROL_RX | UART_FLOWCONTROL_TX);
    // Enable UART1, this call also enables the FIFO buffer necessary for HW flow control
    UARTEnable(UART1_BASE);

}

