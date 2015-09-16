#include <signal.h>

/*
 * Utility-function that reboots the device.
 */
void rebootDevice()
{
    info_log("Rebooting the system.");
    system("/sbin/reboot");
}


/*
 * This method does the initialization, that is needed on a global-basis.
 * All code in this method runs right at the beginning.
 */
void bootstrapInit()
{
    /* VERY IMPORTANT: If this is not done, the "write" on an invalid socket will cause program-crash */
    signal(SIGPIPE, SIG_IGN);
}
