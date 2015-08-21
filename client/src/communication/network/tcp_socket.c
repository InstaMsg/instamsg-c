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

#include "instamsg_vendor.h"
#include "../../common/include/globals.h"
#include "../../common/include/instamsg.h"


static void release_underlying_medium_guaranteed(Network* network)
{
    close(network->socket);
}


static void connect_underlying_medium_try_once(Network* network, unsigned char *hostName, int port)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;

	int rc = SUCCESS;

	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(hostName, NULL, &hints, &result)) == 0)
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
			address.sin_port = htons(port);
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
                    instaMsg.singletonUtilityTimer.startAndCountdownTimer(&(instaMsg.singletonUtilityTimer), 1);
                    return;
                }
                else
                {
                    network->socketCorrupted = 0;
                }
		    }
        }
	}


    // Set timeout-limitation in the socket-receiving function
    struct timeval interval = {NETWORK_READ_TIMEOUT_SECS, 0};
    setsockopt(network->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

    info_log("TCP-SOCKET UNDERLYING_MEDIUM INITIATED FOR HOST = [%s], PORT = [%d].",
             network->host, network->port);
}


static int tcp_socket_read(Network* network, unsigned char* buffer, int len, unsigned char guaranteed)
{
    if(len == 0)
    {
        return SUCCESS;
    }

	int bytes = 0;
    int rc = 0;

    unsigned int errBackup;
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


static int tcp_socket_write(Network* network, unsigned char* buffer, int len)
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


void init_network(Network *network, const char *hostName, unsigned int port)
{
    // Register read-callback.
	network->read = tcp_socket_read;

    // Register write-callback.
	network->write = tcp_socket_write;

    // Keep a copy of connection-parameters, for easy book-keeping.
    SG_MEMSET(network->host, 0, MAX_BUFFER_SIZE)
    sg_sprintf(network->host, "%s", hostName);
    network->port = port;

    // Connect the medium (socket).
    connect_underlying_medium_try_once(network, network->host, network->port);
}


void release_network(Network *network)
{
    release_underlying_medium_guaranteed(network);

    info_log("COMPLETE [TCP-SOCKET] STRUCTURE, INCLUDING THE UNDERLYING MEDIUM (SOCKET) CLEANED FOR HOST = [%s], PORT = [%d].",
             network->host, network->port);
}
