/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "device_defines.h"

#include "../driver/include/cron.h"

#if CRON_ENABLED == 1

/*
 * Here, #task# is the actual task that needs to be run.
 *
 * For example, if the cron-tasklist (as per the config "CRON") is ::
 *
 *          *:*:18:10-20:GPIO/DO001,*:*:20:*:GPIO/DO011
 *
 * Then code will land here every minute from 6:10 PM to 6:20 PM, with #task# == "GPIO/DO001".
 * Also, code will land here every minute from 8:00 PM to 8:59 PM, with #task# == "GPIO/DO011".
 *
 * The device-implementors need to take suitable actions for the tasks (namely, "GPIO/DO001" and "GPIO/DO011" in our example cases).
 */
void runCronTask(char *task)
{
}
#endif

