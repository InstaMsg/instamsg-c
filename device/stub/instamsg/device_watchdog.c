/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

/*
 * This method initializes the watchdog-timer.
 */
void watchdog_init()
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
 * The device must then be reset/restarted.
 *
 * b)
 * "watch_dog_disable()" is called.
 *
 * In this case, the countdown-timer stops, and the device must never be reset/restarted (until the entire
 * "watch_dog_reset_and_enable" loop is repeated).
 *
 */
void watchdog_reset_and_enable(int n, char *callee, void * (*func)(void *), void *arg)
{
}


/*
 * This method disables the watchdog-timer.
 */
void watchdog_disable()
{
}
