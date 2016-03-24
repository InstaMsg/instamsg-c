#include "./include/socket.h"
#include "./include/globals.h"
#include "./include/log.h"
#include "./include/json.h"
#include "./include/watchdog.h"
#include "./include/config.h"

#define SMS     PROSTR("SMS")

static char sms[200];


static void replaceSmsCharacter(char *sms, char old_char, char new_char)
{
    {
        int i;
        for(i = 0; i < strlen(sms); i++)
        {
            if(sms[i] == old_char)
            {
                sms[i] = new_char;
            }
        }

        for(i = strlen(sms) - 1; i >= 0; i--)
        {
            if(sms[i] == ')')
            {
                sms[i] = '}';
            }
        }
    }
}


static void* emptyCallback(void *arg)
{
    return NULL;
}


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

#if GSM_INTERFACE_ENABLED == 1
    /* Empty-initialize the GSM-params. */
    memset(socket->gsmApn, 0, sizeof(socket->gsmApn));
    memset(socket->gsmUser, 0, sizeof(socket->gsmUser));
    memset(socket->gsmPass, 0, sizeof(socket->gsmPass));
    memset(socket->gsmPin, 0, sizeof(socket->gsmPin));
    memset(socket->provPin, 0, sizeof(socket->provPin));

    /* Fill-in the provisioning-parameters. */
    memset(sms, 0, sizeof(sms));

    while(strlen(sms) == 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("\n\n\nProvisioning-SMS not available, retrying to fetch from storage-area\n\n\n"));
        info_log(LOG_GLOBAL_BUFFER);

        startAndCountdownTimer(5, 1);

        watchdog_reset_and_enable(180, "SMS-SCANNING-FOR-PROVISIONG-SMS", 0);
        get_latest_sms_containing_substring(socket, sms, "\"sg_apn\":\"");
        watchdog_disable(emptyCallback, NULL);

        if(strlen(sms) == 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER,
                       PROSTR("\n\n\nProvisioning-SMS not available from storage-area, trying to fetch from config\n\n\n"));
            info_log(LOG_GLOBAL_BUFFER);

#if 0
            sg_sprintf(LOG_GLOBAL_BUFFER, "Giving 10 seconds to insert back the sim-card to test failed-sms-received procedure");
            info_log(LOG_GLOBAL_BUFFER);

            startAndCountdownTimer(10, 1);
#endif

            {
                memset(messageBuffer, 0, sizeof(messageBuffer));

                {
                    int rc = get_config_value_from_persistent_storage(SMS, messageBuffer, sizeof(messageBuffer));
                    if(rc == SUCCESS)
                    {

                        getJsonKeyValueIfPresent(messageBuffer, CONFIG_VALUE_KEY, sms);
                        if(strlen(sms) > 0)
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER, "\n\nProvisioning-Info SMS extracted from config = [%s]\n\n", sms);
                            info_log(LOG_GLOBAL_BUFFER);
                        }
                    }
                }
            }
        }
        else
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "\n\nProvisioning-Info SMS extracted from storage-area = [%s]\n\n", sms);
            info_log(LOG_GLOBAL_BUFFER);

            /*
             * Store this into config, after removing any curly-braces, as config does not support nested-json.
             */
            replaceSmsCharacter(sms, '{', '(');
            replaceSmsCharacter(sms, '}', ')');
            replaceSmsCharacter(sms, '\'', '@');
            replaceSmsCharacter(sms, '"', '#');

            {
                memset(messageBuffer, 0, sizeof(messageBuffer));
                generate_config_json(messageBuffer, SMS, CONFIG_STRING, sms, "");
                save_config_value_on_persistent_storage(SMS, messageBuffer, 1);

                sg_sprintf(LOG_GLOBAL_BUFFER, "Persisted [%s] ==> [%s]", SMS, sms);
                info_log(LOG_GLOBAL_BUFFER);
            }
        }

#if 0
        sms[0] = 0;
#endif
    }


    /*
     * Convert back the simple-brackets to curly-braces.
     */
    replaceSmsCharacter(sms, '(', '{');
    replaceSmsCharacter(sms, ')', '}');
    replaceSmsCharacter(sms, '@', '\'');
    replaceSmsCharacter(sms, '#', '"');

    sg_sprintf(LOG_GLOBAL_BUFFER, "\n\nFinal (Converted) SMS being used for variables-extraction = [%s]\n\n", sms);
    info_log(LOG_GLOBAL_BUFFER);

    getJsonKeyValueIfPresent(sms, "sg_apn", socket->gsmApn);
    getJsonKeyValueIfPresent(sms, "sg_user", socket->gsmUser);
    getJsonKeyValueIfPresent(sms, "sg_pass", socket->gsmPass);
    getJsonKeyValueIfPresent(sms, "sg_pin", socket->gsmPin);
    getJsonKeyValueIfPresent(sms, "prov_pin", socket->provPin);

    sg_sprintf(LOG_GLOBAL_BUFFER,
               PROSTR("\n\nProvisioning-Params ::  sg_apn : [%s], sg_user : [%s], sg_pass : [%s], sg_pin : [%s], prov_pin : [%s]\n\n"),
               socket->gsmApn, socket->gsmUser, socket->gsmPass, socket->gsmPin, socket->provPin);
    info_log(LOG_GLOBAL_BUFFER);

    startAndCountdownTimer(3, 0);
#endif

    /* Connect the medium (socket). */
    watchdog_reset_and_enable(300, "TRYING-SOCKET-CONNECTION-SOLITARY-ATTEMPT", 1);
    connect_underlying_socket_medium_try_once(socket);
    watchdog_disable(NULL, NULL);
}


void release_socket(Socket *socket)
{
    release_underlying_socket_medium_guaranteed(socket);

    sg_sprintf(LOG_GLOBAL_BUFFER,
               PROSTR("COMPLETE [TCP-SOCKET] STRUCTURE, INCLUDING THE UNDERLYING MEDIUM CLEANED FOR HOST = [%s], PORT = [%d]."),
               socket->host, socket->port);
    info_log(LOG_GLOBAL_BUFFER);
}
