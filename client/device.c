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
#if 1
    info_log("sending command to gprs %d", sizeof(command));
    bzero(command, MAX_BUFFER_SIZE);
    //memset(command, 0, MAX_BUFFER_SIZE);
    strcpy((char*)command, "ajay pankaj testing");

    UARTSend(UART3_BASE, command, strlen((char*)command));
    info_log("sent command [%s] to gprs of length [%d]", command, strlen((char*)command));

#if 0
    memset(command, 0, MAX_BUFFER_SIZE);
    strcpy((char*)command, "AT\r\n");

    UARTSend(UART1_BASE, command, strlen((char*)command));
    info_log("sent command [%s] to gprs of length [%d]", command, strlen((char*)command));
#endif

 
    memset(result, 0, MAX_BUFFER_SIZE);
    UARTRecv(UART3_BASE, result, 10000, NO_TIMEOUT);
#endif


    //start("920dfd80-2eef-11e5-b031-34689524378f", "ajaygarg456", NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, NULL);
}
#endif

