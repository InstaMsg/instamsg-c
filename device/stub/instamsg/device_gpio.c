/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "device_defines.h"

#include "../driver/include/gpio.h"

#if SEND_GPIO_INFORMATION == 1

/*
 * Device-implementors need to fill in data for GPIO-pins.
 *
 * For example, if there are 3 Digital-Input pins with status on, on, off; and 2 Digital-Output pins with status off, on,
 * the data will be of form ::
 *
 * GPIO/DI001,DI011,DI020,DO000,DO011
 *
 */
void fill_dio_data(char *buffer, int maxBufferLength)
{
}
#endif


/*
 * Here, "action" will be of the form :;
 *
 *      GPIO/DOxxy
 *
 * Examples ::
 *
 *      GPIO/DO000
 *      GPIO/DO001
 *      GPIO/DO011 .. and so on
 *
 * xx represent the digital-output number
 * y  represents the intended-state (0 means to put off, 1 means to put on.
 */
void performDigitalOutputAction(const char *action)
{
}

