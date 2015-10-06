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
 * All code in this method runs right at the beginning (followed by logger-initialization).
 */
void bootstrapInit()
{
    /* VERY IMPORTANT: If this is not done, the "write" on an invalid socket will cause program-crash */
    signal(SIGPIPE, SIG_IGN);
}


/*
 * This method returns the client-session-data, in simple JSON form, of type ::
 *
 * {key1 : value1, key2 : value2 .....}
 */
void get_client_session_data(char *messageBuffer, int maxBufferLength)
{
}


/*
 * This method returns the client-metadata, in simple JSON form, of type ::
 *
 * {key1 : value1, key2 : value2 .....}
 */
void get_client_metadata(char *messageBuffer, int maxBufferLength)
{
}


/*
 * This method returns the client-network-data, in simple JSON form, of type ::
 *
 * {key1 : value1, key2 : value2 .....}
 */
void get_network_data(char *messageBuffer, int maxBufferLength)
{
}


/*
 * This method gets the device-manufacturer.
 */
void get_manufacturer(char *messagebuffer, int maxbufferlength)
{
}


/*
 * This method gets the device serial-number.
 */
void get_serial_number(char *messagebuffer, int maxbufferlength)
{
}


