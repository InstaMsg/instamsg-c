#include "../driver/include/globals.h"

#ifdef GSM_INTERFACE_ENABLED
/*
 * This method returns the *****LATEST****** sms, which contains the desired substring.
 *
 * Note that "{" are sometimes not processed correctly by some SIMs, so a prefix-match (which
 * otherwise is a stronger check) is not being done.
 *
 * Please note that this method is called by Instamsg-application, *****BEFORE***** calling
 * "connect_underlying_network_medium_try_once".
 */
void get_latest_sms_containing_substring(Network *network, char *buffer, const char *prefix)
{
}
#endif


/*
 * This method returns the univerally-unique-identifier for this device.
 */
void get_device_uuid(char *buffer)
{
}


/*
 * This method tries to establish the network/socket to "network->host" on "network->port".
 *
 * If the connection is successful, then the following must be done by the device-implementation ::
 *                          network->socketCorrupted = 0;
 *
 * Setting the above value will let InstaMsg know that the connection can be used fine for writing/reading.
 */
void connect_underlying_network_medium_try_once(Network* network)
{
}


/*
 * This method reads "len" bytes from network into "buffer".
 *
 * Exactly one of the cases must hold ::
 *
 * a)
 * "guaranteed" is 1.
 * So, this "read" must bahave as a blocking-read.
 *
 * Also, exactly "len" bytes are read successfully.
 * So, SUCCESS must be returned.
 *
 *                      OR
 *
 * b)
 * "guaranteed" is 1.
 * So, this "read" must bahave as a blocking-read.
 *
 * However, an error occurs while reading.
 * So, FAILURE must be returned immediately (i.e. no socket-reinstantiation must be done in this method).
 *
 *                      OR
 *
 * c)
 * "guaranteed" is 0.
 * So, this "read" must behave as a non-blocking read.
 *
 * Also, no bytes could be read in NETWORK_READ_TIMEOUT_SECS seconds (defined in "globals.h").
 * So, SOCKET_READ_TIMEOUT must be returned immediately.
 *
 *                      OR
 *
 * d)
 * "guaranteed" is 0.
 * So, this "read" must behave as a non-blocking read.
 *
 * Also, exactly "len" bytes are successfully read.
 * So, SUCCESS must be returned.
 *
 *                      OR
 *
 * e)
 * "guaranteed" is 0.
 * So, this "read" must behave as a non-blocking read.
 *
 * However, an error occurs while reading.
 * So, FAILURE must be returned immediately (i.e. no socket-reinstantiation must be done in this method).
 */
int network_read(Network* network, unsigned char* buffer, int len, unsigned char guaranteed)
{
    return FAILURE;
}


/*
 * This method writes first "len" bytes from "buffer" onto the network.
 *
 * This is a blocking function. So, either of the following must hold true ::
 *
 * a)
 * All "len" bytes are written.
 * In this case, SUCCESS must be returned.
 *
 *                      OR
 * b)
 * An error occurred while writing.
 * In this case, FAILURE must be returned immediately (i.e. no socket-reinstantiation must be done in this method).
 */
int network_write(struct Network* network, unsigned char* buffer, int len)
{
    return FAILURE;
}


/*
 * This method does the cleaning up (for eg. closing a socket) when the network is cleaned up.
 * But if it is ok to re-connect without releasing the underlying-system-resource, then this can be left empty.
 *
 * Note that this method MUST DO """ONLY""" per-socket level cleanup, NO GLOBAL-LEVEL CLEANING/REINIT MUST BE DONE.
 */
void release_underlying_network_medium_guaranteed(Network* network)
{
}
