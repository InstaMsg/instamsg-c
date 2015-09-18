#include "./include/network.h"
#include "./include/globals.h"
#include "./include/log.h"
#include "./include/json.h"


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
    memset(network->gsmClientId, 0, MAX_CLIENT_ID_SIZE);
    memset(network->gsmAuth, 0, MAX_GSM_PROVISION_PARAM_SIZE);
    memset(network->gsmApn, 0, MAX_GSM_PROVISION_PARAM_SIZE);
    memset(network->gsmUser, 0, MAX_GSM_PROVISION_PARAM_SIZE);
    memset(network->gsmPass, 0, MAX_GSM_PROVISION_PARAM_SIZE);

    /* Fill-in the provisioning-parameters from the SMS obtained from InstaMsg-Server */
    RESET_GLOBAL_BUFFER;
    while(strlen((char*)GLOBAL_BUFFER) == 0)
    {
        info_log("\n\n\nProvisioning-SMS not available, retrying to fetch from storage area\n\n\n");
        startAndCountdownTimer(5, 1);

        get_latest_sms_containing_substring(network, (char*)GLOBAL_BUFFER, "\"cid\":\"");
    }

    /*
     * For some SIMs, the "{" and "}" sent from server are replaced by "(" and ")".
     * Rectify them.
     */
    {
        int i;
        for(i = 0; i < strlen((char*)GLOBAL_BUFFER); i++)
        {
            if(GLOBAL_BUFFER[i] == '(')
            {
                GLOBAL_BUFFER[i] = '{';
                break;
            }
        }

        for(i = strlen((char*)GLOBAL_BUFFER) - 1; i >= 0; i--)
        {
            if(GLOBAL_BUFFER[i] == ')')
            {
                GLOBAL_BUFFER[i] = '}';
                break;
            }
        }

    }

    getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, "cid", network->gsmClientId);
    getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, "auth", network->gsmAuth);
    getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, "apn", network->gsmApn);
    getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, "user", network->gsmUser);
    getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, "pass", network->gsmPass);

    info_log("\n\nProvisioning-Params ::  cid : [%s], auth : [%s], apn : [%s], user : [%s], pass : [%s]\n\n",
             network->gsmClientId, network->gsmAuth, network->gsmApn, network->gsmUser, network->gsmPass);
    startAndCountdownTimer(3, 0);
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
