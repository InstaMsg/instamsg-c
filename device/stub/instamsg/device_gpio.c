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

