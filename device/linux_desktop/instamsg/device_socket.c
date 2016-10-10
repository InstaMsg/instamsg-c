/*******************************************************************************
 *
 * Copyright (c) 2014 SenseGrow, Inc.
 *
 * SenseGrow Internet of Things (IoT) Client Frameworks
 * http://www.sensegrow.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Contributors:
 *    Ajay Garg <ajay.garg@sensegrow.com>
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
#include <unistd.h>

#include "../driver/include/globals.h"
#include "../driver/include/socket.h"
#include "../driver/include/log.h"

extern unsigned int bytes_sent_over_wire;
extern unsigned int bytes_received_over_wire;

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
void get_latest_sms_containing_substring(Socket *socket, char *buffer, const char *substring)
{
    read_singular_line_from_file(SMS_FILE, "latest-sms", buffer, 200);
    strip_leading_and_trailing_white_paces(buffer);
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
void connect_underlying_socket_medium_try_once(Socket* s)
{
	int type = -1;
	struct sockaddr_in address;

	int rc = SUCCESS;
    struct timeval interval = {SOCKET_READ_TIMEOUT_SECS, 0};

	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

    if(strcmp(s->type, SOCKET_TCP) == 0)
    {
        type = SOCK_STREAM;
    }
    else if(strcmp(s->type, SOCKET_UDP) == 0)
    {
        type = SOCK_DGRAM;
    }

	if ((rc = getaddrinfo(s->host, NULL, &hints, &result)) == 0)
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
			address.sin_port = htons(s->port);
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
		    s->socket = socket(family, type, 0);
		    if (s->socket != -1)
		    {
			    int opt = 1;
                if(connect(s->socket, (struct sockaddr*)&address, sizeof(address)) != 0)
                {
                    sg_sprintf(LOG_GLOBAL_BUFFER, SOCKET_NOT_AVAILABLE);
                    info_log(LOG_GLOBAL_BUFFER);

                    startAndCountdownTimer(1, 0);

                    return;
                }
                else
                {
                    /*
                     * VERY IMPORTANT.. MUST BE DONE.
                     */
                    s->socketCorrupted = 0;
                }
		    }
        }
	}


    /* Set timeout-limitation in the socket-receiving function */
    setsockopt(s->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

    sg_sprintf(LOG_GLOBAL_BUFFER, "%s-SOCKET UNDERLYING_MEDIUM INITIATED FOR HOST = [%s], PORT = [%d].",
             s->type, s->host, s->port);
    info_log(LOG_GLOBAL_BUFFER);
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
 * Also, no bytes could be read in SOCKET_READ_TIMEOUT_SECS seconds (defined in "globals.h").
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
int socket_read(Socket* socket, unsigned char* buffer, int len, unsigned char guaranteed)
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
		rc = recv(socket->socket, &buffer[bytes], (size_t)(len - bytes), 0);
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
                    sg_sprintf(LOG_GLOBAL_BUFFER, "%sTimeout occurred while waiting for data.. retrying", SOCKET_READ);
                    debug_log(LOG_GLOBAL_BUFFER);

                    continue;
                }
                else
                {
                    /*
                     * WE have genuinely timed-out.
                     * Return this info, and let the calling-function take appropriate action.
                     */
                    sg_sprintf(LOG_GLOBAL_BUFFER, "%sTimeout occurred while waiting for data.. NOT retrying", SOCKET_READ);
                    debug_log(LOG_GLOBAL_BUFFER);

                    return SOCKET_READ_TIMEOUT; /* Case c) */
                }
            }
            else
            {
                /*
                 * There was some error on the socket.
                 */
                sg_sprintf(LOG_GLOBAL_BUFFER, "%sErrno [%d] occurred while reading from socket", SOCKET_READ, errBackup);
                error_log(LOG_GLOBAL_BUFFER);

                return FAILURE; /* Case b) and e) */
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
                sg_sprintf(LOG_GLOBAL_BUFFER, "%sErrno [%d] occurred while reading from socket", SOCKET_READ, errBackup);
                error_log(LOG_GLOBAL_BUFFER);

                return FAILURE; /* Another leg of case b) and e) */
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

    bytes_received_over_wire = bytes_received_over_wire + len;
    return SUCCESS; /* Case a) and d) */
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
int socket_write(Socket* socket, unsigned char* buffer, int len)
{
    int bytes = 0;
    int rc = 0;

    unsigned int errBackup;
    while(bytes < len)
    {
	    rc = write(socket->socket, &buffer[bytes], (size_t)(len - bytes));
        errBackup = errno;

        if(rc < 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "%sErrno [%d] occurred while writing to socket", SOCKET_WRITE, errno);
            error_log(LOG_GLOBAL_BUFFER);

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
                sg_sprintf(LOG_GLOBAL_BUFFER, "%sErrno [%d] occurred while sending to socket", SOCKET_WRITE, errBackup);
                error_log(LOG_GLOBAL_BUFFER);

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

    bytes_sent_over_wire = bytes_sent_over_wire + len;
    return SUCCESS;
}


/*
 * This method does the cleaning up (for eg. closing a socket) when the socket is cleaned up.
 * But if it is ok to re-connect without releasing the underlying-system-resource, then this can be left empty.
 *
 * Note that this method MUST DO """ONLY""" per-socket level cleanup, NO GLOBAL-LEVEL CLEANING/REINIT MUST BE DONE.
 */
void release_underlying_socket_medium_guaranteed(Socket* socket)
{
#if SSL_ENABLED == 1
    resetDevice();
#else
    close(socket->socket);
#endif
}
