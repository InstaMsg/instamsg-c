#include "./include/network.h"
#include "./include/globals.h"
#include "./include/log.h"

void init_network(Network *network, const char *hostName, unsigned int port)
{
    /* Register read-callback. */
	network->read = network_read;

    /* Register write-callback. */
	network->write = network_write;

    /* Keep a copy of connection-parameters, for easy book-keeping. */
    memset(network->host, 0, MAX_BUFFER_SIZE);
    sg_sprintf(network->host, "%s", hostName);
    network->port = port;

#ifdef GSM_INTERFACE_ENABLED
    /* Empty-initialize the GSM-params. */
    memset(network->gsmClientId, 0, MAX_GSM_PROVISION_PARAM_SIZE);
    memset(network->gsmApn, 0, MAX_GSM_PROVISION_PARAM_SIZE);
    memset(network->gsmUser, 0, MAX_GSM_PROVISION_PARAM_SIZE);
    memset(network->gsmPass, 0, MAX_GSM_PROVISION_PARAM_SIZE);
#endif

    /* Connect the medium (socket). */
    connect_underlying_network_medium_try_once(network);
}


void release_network(Network *network)
{
    release_underlying_network_medium_guaranteed(network);

    info_log("COMPLETE [TCP-SOCKET] STRUCTURE, INCLUDING THE UNDERLYING MEDIUM CLEANED FOR HOST = [%s], PORT = [%d].",
             network->host, network->port);
}
