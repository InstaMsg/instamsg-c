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
 *
 * THIS METHOD IS GENERALLY EXPECTED TO BE EMPTY, AS THE INITIALIZATION SHOULD HAPPEN ON
 * PER-MODULE BASIS. ONLY IF SOMETHING IS REALLY NEEDED RIGHT AT THE START (WHICH IS THEN
 * POTENTIALLY USED BY MORE THAN ONE MODULE), SHOULD SOMETHING BE REQUIRED TO COME HERE.
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
 * This method returns the univerally-unique-identifier for this device.
 */
void get_device_uuid(char *buffer, int maxbufferlength)
{
}
