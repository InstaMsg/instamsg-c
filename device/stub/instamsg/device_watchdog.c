/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

/*
 * This method initializes the watchdog-timer.
 */
void init_watchdog()
{
}


/*
 * This method resets the watchdog-timer.
 *
 * Once this is completed, the watchdog-timer starts counting down from "n" seconds to 0.
 * Then either of the following must happen ::
 *
 * a)
 * Counter reaches 0.
 *
 * In this case, the global "watchdog_expired" variable must be set to 1.
 * Also, if "immediate" is 1, the device must be reboooted immediately.
 *
 * b)
 * "watch_dog_disable()" (the global API-function) is called by the callee.
 *
 * In this case, the countdown-timer stops, and the device must not be reset/restarted.
 *
 */
void do_watchdog_reset_and_enable(int n, unsigned char immediate)
{
}


/*
 * This method disables the watchdog-timer.
 */
void do_watchdog_disable()
{
}
