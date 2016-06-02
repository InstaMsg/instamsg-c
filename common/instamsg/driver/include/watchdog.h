#ifndef INSTAMSG_WATCHDOG_COMMON
#define INSTAMSG_WATCHDOG_COMMON

void watchdog_init();
void do_watchdog_reset_and_enable(int n, unsigned char immediate);
void do_watchdog_disable();
void print_rebooting_message();

extern volatile unsigned char watchdog_active;
extern volatile unsigned char watchdog_expired;




/*
 *********************************************************************************************************************
 **************************************** PUBLIC APIs *****************************************************************
 **********************************************************************************************************************
 */

/*
 * This method starts the watchdog timer, passing various arguments in the process ::
 *
 * n                    :
 *
 *      Number of seconds for which the watchdog must be run.
 *
 *
 * callee               :
 *
 *      A string to identify the callee-method.
 *      This is merely for informational/debugging purposes.
 *
 *
 * immediate            :
 *
 *      A flag to denote whether the device should be rebooted immediately upon watchdog expiry (indispensable in certain situations).
 *
 *      Value of 1 denotes immediate restart.
 *      Value of 0 denotes not-immediate restart (in this case, device will be rebooted as per the case in "watchdog_disable").
 *
 */
void watchdog_reset_and_enable(int n, char *callee, unsigned char immediate);






/*
 * This method disables the watchdog.
 *
 *
 * Following are the parameters ::
 *
 * func                 :
 *
 *      A callback-function that is called, if the watchdog runs to completion (that is, the watchdog has expired).
 *
 *
 * arg                  :
 *
 *      Argument to the "func" callback.
 *
 *
 *
 * Following is the behaviour ::

 * i)
 * If "watchdog_expired" remains 0, nothing happens.
 *
 * ii)
 * If "watchdog_expired" has become "1" and "func" is NULL, the device is rebooted (via call to "rebootDevice").
 *
 * iii)
 * If "watchdog_expired" has become "1" and "func" is not NULL, then "func" is called, and it is the responsibility of "func" to
 * decide whether to reboot the device or not (and actually reboot the device is required).
 *
 */
void watchdog_disable(void * (*func)(void *), void *arg);






/*
 * This method tells whether the watchdog has expired or not.
 *
 * If it has, the method returns 1.
 * If not, the method returns 0.
 */
unsigned char time_fine_for_time_limit_function();

#endif
