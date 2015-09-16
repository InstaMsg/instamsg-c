/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "../common/include/globals.h"
#include "../common/include/instamsg.h"

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
	int type = SOCK_STREAM;
	struct sockaddr_in address;

	int rc = SUCCESS;
    struct timeval interval = {NETWORK_READ_TIMEOUT_SECS, 0};

	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(network->host, NULL, &hints, &result)) == 0)
	{
		struct addrinfo* res = result;

		/* prefer ip4 addresses */
		while (res)
		{
			if (res->ai_family == AF_INET)
			{
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET)
		{
			address.sin_port = htons(network->port);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else
			rc = FAILURE;

		freeaddrinfo(result);
	}

	if (rc == SUCCESS)
	{
        if(1)
        {
		    network->socket = socket(family, type, 0);
		    if (network->socket != -1)
		    {
			    int opt = 1;
                if(connect(network->socket, (struct sockaddr*)&address, sizeof(address)) != 0)
                {
                    info_log(NETWORK_NOT_AVAILABLE);
                    startAndCountdownTimer(1, 0);

                    return;
                }
                else
                {
                    /*
                     * VERY IMPORTANT.. MUST BE DONE.
                     */
                    network->socketCorrupted = 0;
                }
		    }
        }
	}


    /* Set timeout-limitation in the socket-receiving function */
    setsockopt(network->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

    info_log("TCP-SOCKET UNDERLYING_MEDIUM INITIATED FOR HOST = [%s], PORT = [%d].",
             network->host, network->port);
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
	int bytes = 0;
    int rc = 0;
    unsigned int errBackup;

    if(len == 0)
    {
        return SUCCESS;
    }


	while (bytes < len)
	{
		rc = recv(network->socket, &buffer[bytes], (size_t)(len - bytes), 0);
        errBackup = errno;

        if(rc < 0)
        {
            if((errBackup == EAGAIN) || (errBackup == EWOULDBLOCK))
            {
                /*
                 * Timeout occurred before we could read any bytes.
                 * Now, we re-read the bytes if we are in guaranteed (pseudo-blocking) mode.
                 */
                if(guaranteed == 1)
                {
                    debug_log(SOCKET_READ "Timeout occurred while waiting for data.. retrying");
                    continue;
                }
                else
                {
                    /*
                     * WE have genuinely timed-out.
                     * Return this info, and let the calling-function take appropriate action.
                     */
                    debug_log(SOCKET_READ "Timeout occurred while waiting for data.. NOT retrying");
                    return SOCKET_READ_TIMEOUT;
                }
            }
            else
            {
                /*
                 * There was some error on the socket.
                 */
                error_log(SOCKET_READ "Errno [%d] occurred while reading from socket", errBackup);
                return FAILURE;
            }
        }

        /* STRANGE:
         *
         * 1)
         * On Ubuntu 14.04, if "n" bytes are received successfully as one chunk, rc is 0 (and not "n") :(
         *
         * 2)
         * Moreover, rc is 0, in both the following cases ::
         *
         *      i)  All n bytes were successfully received.
         *      ii) Peer shutdown the socket
         *
         * So, the following hacky test has been deployed
         */
        if(rc == 0)
        {
            if(errBackup != 0)
            {
                error_log(SOCKET_READ "Errno [%d] occurred while reading from socket", errBackup);
                return FAILURE;
            }
            else
            {
                bytes = len;
            }
        }
        else
        {
            bytes = bytes + rc;
        }
	}

    return SUCCESS;
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
int network_write(Network* network, unsigned char* buffer, int len)
{
    int bytes = 0;
    int rc = 0;

    unsigned int errBackup;
    while(bytes < len)
    {
	    rc = write(network->socket, &buffer[bytes], (size_t)(len - bytes));
        errBackup = errno;

        if(rc < 0)
        {
            error_log(SOCKET_WRITE "Errno [%d] occurred while writing to socket", errno);
            return FAILURE;
        }

        /* STRANGE:
         *
         * 1)
         * On Ubuntu 14.04, if "n" bytes are received successfully as one chunk, rc is 0 (and not "n") :(
         *
         * 2)
         * Moreover, rc is 0, in both the following cases ::
         *
         *      i)  All n bytes were successfully sent.
         *      ii) Peer shutdown the socket
         *
         * So, the following hacky test has been deployed
         */
        if(rc == 0)
        {
            if(errBackup != 0)
            {
                error_log(SOCKET_READ "Errno [%d] occurred while sending to socket", errBackup);
                return FAILURE;
            }
            else
            {
                bytes = len;
            }
        }
        else
        {
            bytes = bytes + rc;
        }
	}

    return SUCCESS;
}


/*
 * This method does the cleaning up (for eg. closing a socket) when the network is cleaned up.
 *
 * Note that this method MUST DO """ONLY""" per-socket level cleanup, NO GLOBAL-LEVEL CLEANING/REINIT MUST BE DONE.
 */
void release_underlying_network_medium_guaranteed(Network* network)
{
    close(network->socket);
}
