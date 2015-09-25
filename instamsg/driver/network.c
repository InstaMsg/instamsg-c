#include "./include/network.h"
#include "./include/globals.h"
#include "./include/log.h"
#include "./include/json.h"


static char sms[200];

void init_network(Network *network, const char *hostName, unsigned int port)
{
    /* Register read-callback. */
	network->read = network_read;

    /* Register write-callback. */
	network->write = network_write;

    /* Keep a copy of connection-parameters, for easy book-keeping. */
    memset(network->host, 0, sizeof(network->host));
    sg_sprintf(network->host, "%s", hostName);
    network->port = port;

#ifdef GSM_INTERFACE_ENABLED
    /* Empty-initialize the GSM-params. */
    memset(network->gsmApn, 0, sizeof(network->gsmApn));
    memset(network->gsmUser, 0, sizeof(network->gsmUser));
    memset(network->gsmPass, 0, sizeof(network->gsmPass));

    /* Fill-in the provisioning-parameters from the SMS obtained from InstaMsg-Server */
    memset(sms, 0, sizeof(sms));
    while(strlen(sms) == 0)
    {
        info_log("\n\n\nProvisioning-SMS not available, retrying to fetch from storage area\n\n\n");
        startAndCountdownTimer(5, 1);

        get_latest_sms_containing_substring(network, sms, "\"sg_apn\":\"");
    }

    /*
     * For some SIMs, the "{" and "}" sent from server are replaced by "(" and ")".
     * Rectify them.
     */
    {
        int i;
        for(i = 0; i < strlen(sms); i++)
        {
            if(sms[i] == '(')
            {
                sms[i] = '{';
                break;
            }
        }

        for(i = strlen(sms) - 1; i >= 0; i--)
        {
            if(sms[i] == ')')
            {
                sms[i] = '}';
                break;
            }
        }

    }

    getJsonKeyValueIfPresent(sms, "sg_apn", network->gsmApn);
    getJsonKeyValueIfPresent(sms, "sg_user", network->gsmUser);
    getJsonKeyValueIfPresent(sms, "sg_pass", network->gsmPass);

    info_log("\n\nProvisioning-Params ::  sg_apn : [%s], sg_user : [%s], sg_pass : [%s]\n\n",
             network->gsmApn, network->gsmUser, network->gsmPass);
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
