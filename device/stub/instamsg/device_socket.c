#include "../driver/include/globals.h"

#include "./device_socket.h"

#if GSM_INTERFACE_ENABLED == 1
/*
 * This method returns the *****LATEST****** sms, which contains the desired substring, while the following returns 1 ::
 *
 *                                      time_fine_for_time_limit_function()
 *
 * Note that "{" are sometimes not processed correctly by some SIMs, so a prefix-match (which
 * otherwise is a stronger check) is not being done.
 *
 * Please note that this method is called by Instamsg-application, *****BEFORE***** calling
 * "connect_underlying_socket_medium_try_once".
 */
void get_latest_sms_containing_substring(SG_Socket *socket, char *buffer, const char *substring)
{
#error "Function not implemented."
}
#endif


/*
 * This method tries to establish the socket/socket to "socket->host" on "socket->port".
 *
 * If the connection is successful, then the following must be done by the device-implementation ::
 *                          socket->socketCorrupted = 0;
 *
 * Setting the above value will let InstaMsg know that the connection can be used fine for writing/reading.
 */
void connect_underlying_socket_medium_try_once(SG_Socket* socket)
{
}


/*
 * This method reads "len" bytes from socket into "buffer".
 *
 * Exactly one of the cases must hold ::
 *
 * a)
 * "guaranteed" is 1.
 * So, this "read" must bahave as a blocking-read.
 *
 * Also, exactly "len" bytes are read successfully.
 * So,
 *
 *      i) "socket->bytes_received" must be set to "len".
 *      ii) SUCCESS must be returned.
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
 * Also, no/partial bytes (but not "len" bytes) could be read in SOCKET_READ_TIMEOUT_SECS seconds (defined in "globals.h").
 * So,
 *
 *      i) "socket->bytes_received" must be set to whatever partial number of bytes are read.
 *      ii) SOCKET_READ_TIMEOUT must be returned.
 *
 *                      OR
 *
 * d)
 * "guaranteed" is 0.
 * So, this "read" must behave as a non-blocking read.
 *
 * Also, exactly "len" bytes are successfully read.
 * So
 *
 *      i) "socket->bytes_received" must be set to "len".
 *      ii) SUCCESS must be returned.
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
int socket_read(SG_Socket* socket, unsigned char* buffer, int len, unsigned char guaranteed)
{
    return FAILURE;
}


/*
 * This method writes first "len" bytes from "buffer" onto the socket.
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
int socket_write(SG_Socket* socket, unsigned char* buffer, int len)
{
    return FAILURE;
}


/*
 * This method does the cleaning up (for eg. closing a socket) when the socket is cleaned up.
 * But if it is ok to re-connect without releasing the underlying-system-resource, then this can be left empty.
 *
 * Note that this method MUST DO """ONLY""" per-socket level cleanup, NO GLOBAL-LEVEL CLEANING/REINIT MUST BE DONE.
 */
void release_underlying_socket_medium_guaranteed(SG_Socket* socket)
{
}


/*
 * This method must JUST close the socket, and nothing else (resetting, etc).
 *
 * Since this method is most likely to be used internally only, so it can be assumed that whenever this method is called,
 * the socket is in fine state.
 */
void release_socket_simple_guaranteed(SG_Socket* socket)
{
}

