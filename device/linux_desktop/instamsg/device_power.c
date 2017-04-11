/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "device_defines.h"

#include "../driver/include/power.h"

#if SEND_POWER_INFORMATION == 1
/*
 * The device may decide how to send the power-information to the server.
 * Or how to consume this string in general.
 */
void send_power_information()
{
}
#endif

