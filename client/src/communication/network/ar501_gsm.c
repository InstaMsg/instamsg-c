/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "instamsg_vendor.h"
#include "../../common/include/globals.h"
#include "../../common/include/instamsg.h"


static void release_underlying_medium_guaranteed(Network* network)
{
}


static void connect_underlying_medium_guaranteed(Network* network, char *hostName, int port)
{
    info_log("TCP-SOCKET UNDERLYING_MEDIUM INITIATED FOR HOST = [%s], PORT = [%d].",
             network->host, network->port);
}


static int ar501_gsm_socket_read(Network* network, unsigned char* buffer, int len, unsigned char guaranteed)
{
    return SUCCESS;
}


static int ar501_gsm_socket_write(Network* network, unsigned char* buffer, int len)
{
    return SUCCESS;
}


void init_network(Network *network, const char *hostName, unsigned int port)
{
    // Register read-callback.
	network->read = ar501_gsm_socket_read;

    // Register write-callback.
	network->write = ar501_gsm_socket_write;

    // Keep a copy of connection-parameters, for easy book-keeping.
    memset(network->host, 0, MAX_BUFFER_SIZE);
    snprintf(network->host, MAX_BUFFER_SIZE - 1, "%s", hostName);
    network->port = port;

    // Connect the medium (socket).
    connect_underlying_medium_guaranteed(network, network->host, network->port);
}


void release_network(Network *network)
{
    release_underlying_medium_guaranteed(network);

    info_log("COMPLETE [TCP-SOCKET] STRUCTURE, INCLUDING THE UNDERLYING MEDIUM (SOCKET) CLEANED FOR HOST = [%s], PORT = [%d].",
             network->host, network->port);
}
