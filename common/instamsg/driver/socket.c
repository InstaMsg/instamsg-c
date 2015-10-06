#include "./include/socket.h"
#include "./include/globals.h"
#include "./include/log.h"
#include "./include/json.h"


static char sms[200];

void init_socket(Socket *socket, const char *hostName, unsigned int port)
{
    /* Register read-callback. */
	socket->read = socket_read;

    /* Register write-callback. */
	socket->write = socket_write;

    /* Keep a copy of connection-parameters, for easy book-keeping. */
    memset(socket->host, 0, sizeof(socket->host));
    sg_sprintf(socket->host, "%s", hostName);
    socket->port = port;

#ifdef GSM_INTERFACE_ENABLED
    /* Empty-initialize the GSM-params. */
    memset(socket->gsmApn, 0, sizeof(socket->gsmApn));
    memset(socket->gsmUser, 0, sizeof(socket->gsmUser));
    memset(socket->gsmPass, 0, sizeof(socket->gsmPass));

    /* Fill-in the provisioning-parameters from the SMS obtained from InstaMsg-Server */
    memset(sms, 0, sizeof(sms));
    while(strlen(sms) == 0)
    {
        info_log("\n\n\nProvisioning-SMS not available, retrying to fetch from storage area\n\n\n");
        startAndCountdownTimer(5, 1);

        get_latest_sms_containing_substring(socket, sms, "\"sg_apn\":\"");
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

    getJsonKeyValueIfPresent(sms, "sg_apn", socket->gsmApn);
    getJsonKeyValueIfPresent(sms, "sg_user", socket->gsmUser);
    getJsonKeyValueIfPresent(sms, "sg_pass", socket->gsmPass);

    info_log("\n\nProvisioning-Params ::  sg_apn : [%s], sg_user : [%s], sg_pass : [%s]\n\n",
             socket->gsmApn, socket->gsmUser, socket->gsmPass);
    startAndCountdownTimer(3, 0);
#endif

    /* Connect the medium (socket). */
    connect_underlying_socket_medium_try_once(socket);
}


void release_socket(Socket *socket)
{
    release_underlying_socket_medium_guaranteed(socket);

    info_log("COMPLETE [TCP-SOCKET] STRUCTURE, INCLUDING THE UNDERLYING MEDIUM CLEANED FOR HOST = [%s], PORT = [%d].",
             socket->host, socket->port);
}