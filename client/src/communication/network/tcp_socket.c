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

#include "instamsg_vendor.h"
#include "../../common/include/config.h"


static void release_underlying_medium_guaranteed(Network* network)
{
    close(network->socket);
}


static void connect_underlying_medium_guaranteed(Network* network)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;

	int rc = SUCCESS;

	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

    char hostName[MAX_BUFFER_SIZE] = {0};
    readConfig(&config, "SERVER_IP", STRING, hostName);
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

            int port;
            readConfig(&config, "SERVER_PORT", INTEGER, &port);

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
        while(1)
        {
		    network->socket = socket(family, type, 0);
		    if (network->socket != -1)
		    {
			    int opt = 1;
                if(connect(network->socket, (struct sockaddr*)&address, sizeof(address)) != 0)
                {
                    info_log("Could not connect to the network ... retrying\n");

                    thread_sleep(1);
                }
                else
                {
                    break;
                }
		    }
        }
	}

    info_log("TCP-SOCKET structure underlying physical-medium initiated.\n");
}


static int tcp_socket_read(Network* network, unsigned char* buffer, int len)
{
	int bytes = 0;
    int rc = 0;

	while (bytes < len)
	{
		while(rc = recv(network->socket, &buffer[bytes], (size_t)(len - bytes), 0) < 0)
        {
            return FAILURE;
        }

        // STRANGE: On Ubuntu 14.04, if "n" bytes are received successfully as one chunk, rc is 0 (and not "n") :(
        if(rc == 0)
        {
            bytes = len;
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

    while(bytes < len)
    {
	    while(rc = write(network->socket, &buffer[bytes], (size_t)(len - bytes)) < 0)
        {
            return FAILURE;
        }

        // STRANGE: On Ubuntu 14.04, if "n" bytes are sent successfully as one chunk, rc is 0 (and not "n") :(
        if(rc == 0)
        {
            bytes = len;
        }
        else
        {
            bytes = bytes + rc;
        }
    }

    return SUCCESS;
}


void init_network(Network *network, void *arg)
{
    // Register read-callback.
	network->read = tcp_socket_read;

    // Register write-callback.
	network->write = tcp_socket_write;

    // Connect the medium (socket).
    connect_underlying_medium_guaranteed(network);
}


void release_network(Network *n)
{
    release_underlying_medium_guaranteed(n);

    info_log("COMPLETE [TCP-SOCKET] STRUCTURE, INCLUDING THE UNDERLYING MEDIUM (SOCKET) CLEANED.\n");
}
