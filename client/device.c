#if 0
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "src/common/include/globals.h"

#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/can.h"

#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3



int main()
{
    char m[100];
    SG_MEMSET(m, 0, 100)

    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN);

   for (;;) {
	// set the red LED pin high, others low
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_BLUE);
	ROM_SysCtlDelay(5000000);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, 0);
	ROM_SysCtlDelay(5000000);
    }

}
#endif

#if 1
#include "uart_utils.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"




#include "src/common/include/instamsg.h"
#include "src/common/include/globals.h"

#include <string.h>

void coreLoopyBusinessLogicInitiatedBySelf()
{
    info_log("Business-Logic Called.");
}



unsigned char result[MAX_BUFFER_SIZE] = {0};
unsigned char command[MAX_BUFFER_SIZE] = {0};
int main(int argc, char** argv)
{
    globalSystemInit();




//////////////////////////////////////////////////////////////
//
    /*
     * Enable UART1.
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    /*
     * Enable RX/TX.
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinConfigure(GPIO_PC4_U1RX);
    GPIOPinConfigure(GPIO_PC5_U1TX);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    /*
     * Enable RTS/CTS.
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinConfigure(GPIO_PF0_U1RTS);
    GPIOPinConfigure(GPIO_PF1_U1CTS);
    GPIOPinTypeUART(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /*
     * Enable the UART-clocking
     */
    //UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200,
                       (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_NONE));

   /*
    * Re-initializing Port-F :(
    */
    //RTS and CTS in IO mode
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);   //RTS output
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);          //CTS input
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);


#if 0
    /*
     * Commenting this part as of now, as it requires FreeRTOS.
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);            //D7 is used for RI detection of GPRS Modem
    GPIOPinIntDisable(GPIO_PORTD_BASE, 0xFF);

    /* This is to enable GPIO capability on Pin PD7... Work around for NMI signal on the pin */
    HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD;
    HWREG(GPIO_PORTD_BASE + GPIO_O_CR) = 0xff;
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_7);
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);
#endif


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);                // DCD pin Monitor
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);                //GPRS Reset IO
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);                //GPRS Power Enable
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);

    ROM_UARTEnable(UART1_BASE);
    ROM_IntEnable(INT_UART1);
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT|UART_INT_CTS);
    //UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_CTS);


//////////////////////////////////////////////////////////////

#if 0



    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);
    ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);



    /*
     * Initialize the UART1.
     */
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    /*
     * Set up the RTS/CTS pins.
     */
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinConfigure(GPIO_PF0_U1RTS);
    GPIOPinConfigure(GPIO_PF1_U1CTS);
    ROM_GPIOPinTypeUART(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /*
     * Set up the RX/TX pins.
     */
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinConfigure(GPIO_PC4_U1RX);
    GPIOPinConfigure(GPIO_PC5_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    /*
     * Configure the UART for 9600, 8-N-1 operation.
     */
    ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    /*
     * Final UART1-initializations.
     */
    ROM_UARTFIFOEnable(UART1_BASE);
    ROM_UARTEnable(UART1_BASE);


    /*
     * Enable the interrupts.
     */
    ROM_IntEnable(INT_UART1);
    UARTIntDisable(UART1_BASE,UART_INT_TX);
    UARTIntEnable(UART1_BASE, UART_INT_RX);



    startAndCountdownTimer(3);
#endif

    startAndCountdownTimer(5);
    memset(command, 0, MAX_BUFFER_SIZE);
    strcpy((char*)command, "AT+CGSN\r\n");
    //strcpy((char*)command, "AT#SIMDET?\r\n");
    //strcpy((char*)command, "AT+CNUM\r\n");
    //strcpy((char*)command, "AT+CPIN?\r\n");
    //strcpy((char*)command, "AT+CREG?\r\n");

    //startAndCountdownTimer(1);
    //                          info_log("%d", ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1));
    UARTSend(UART1_BASE, command, strlen((char*)command));
    //                          info_log("%d", ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1));
    //info_log("sent string [%s] of length [%d]", command, strlen((char*)command));

#if 0
memset(command, 0, MAX_BUFFER_SIZE);
    strcpy((char*)command, "AT+CGMI\r\n");

    startAndCountdownTimer(1);
                              info_log("%d", ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1));
    UARTSend(UART1_BASE, command, strlen((char*)command));
                              info_log("%d", ROM_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1));
    info_log("sent string [%s] of length [%d]", command, strlen((char*)command));
#endif

    //startAndCountdownTimer(2);
    memset(result, 0, MAX_BUFFER_SIZE);


    //UARTRecv(UART1_BASE, result, 10000, NO_TIMEOUT);
    while(1)
    {
    }

    //start("920dfd80-2eef-11e5-b031-34689524378f", "ajaygarg456", NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, NULL);
}
#endif

