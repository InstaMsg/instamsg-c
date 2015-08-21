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
#include "src/common/include/instamsg.h"
#include "src/common/include/globals.h"

void coreLoopyBusinessLogicInitiatedBySelf()
{
    info_log("Business-Logic Called.");
}


int main(int argc, char** argv)
{
    SYSTEM_GLOBAL_INIT();
    start("920dfd80-2eef-11e5-b031-34689524378f", "ajaygarg456", NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, NULL);
}
#endif

