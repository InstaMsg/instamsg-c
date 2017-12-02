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



#include "./include/socket.h"
#include "./include/globals.h"
#include "./include/log.h"
#include "./include/json.h"
#include "./include/watchdog.h"
#include "./include/config.h"
#include "./include/misc.h"
#include "./include/sg_stdlib.h"
#include "./include/sg_mem.h"

#if SSL_ENABLED == 1
#include "./include/wolfssl/internal.h"
#include "./include/wolfssl/error-ssl.h"
#endif


#if GSM_INTERFACE_ENABLED == 1

char sms[200];
static unsigned char smsFetched;

void replaceSmsCharacter(char *sms, char old_char, char new_char);

void replaceSmsCharacter(char *sms, char old_char, char new_char)
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


void store_sms_in_config(char *sms, char *smsConfigBuffer, int smsConfigBufferLength)
{
	char *bufferToUse = NULL;
	int bufferLengthToUse = 0;

	if(1)
    {
    	if(1)
    	{
    		/*
    		 * Store this into config, after removing any curly-braces, as config does not support nested-json.
    		 */
    		replaceSmsCharacter(sms, '{', '(');
    		replaceSmsCharacter(sms, '}', ')');
    		replaceSmsCharacter(sms, '\'', '@');
    		replaceSmsCharacter(sms, '"', '#');

    		if(smsConfigBuffer != NULL)
    		{
    			bufferToUse = smsConfigBuffer;
    			bufferLengthToUse = smsConfigBufferLength;
    		}
    		else
    		{
    			bufferToUse = messageBuffer;
    			bufferLengthToUse = sizeof(messageBuffer);
    		}

    		if(1)
    		{
    			memset(bufferToUse, 0, bufferLengthToUse);
    			generate_config_json(bufferToUse, SMS, CONFIG_STRING, sms, "");
    			save_config_value_on_persistent_storage(SMS, bufferToUse, 0);

    			sg_sprintf(LOG_GLOBAL_BUFFER, "Persisted [%s] ==> [%s]", SMS, sms);
    			info_log(LOG_GLOBAL_BUFFER);
    		}
    	}
    }
}
#endif


#if SSL_ENABLED == 1

/*
 * This method reads "len" bytes from "ssl_bio" into "buffer".
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
 * So, FAILURE must be returned immediately.
 *
 *                      OR
 *
 * c)
 * "guaranteed" is 0.
 * So, this "read" must behave as a non-blocking read.
 *
 * However, no bytes could be read, and neither did any error occur.
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
 * So, FAILURE must be returned immediately.
 */
static int secure_socket_read(SG_Socket* socket, unsigned char* buffer, int len, unsigned char guaranteed)
{
    int rem = len;
    socket->bytes_received = 0;

    while(1)
    {
        int rc = wolfSSL_read(socket->ssl, buffer, len);
        if(rc > 0)
        {
            rem = rem - rc;
            if(rem == 0)
            {
                return SUCCESS;
            }
        }
        else if(rc == WANT_READ)
        {
            return SOCKET_READ_TIMEOUT;
        }
        else
        {
            return FAILURE;
        }
    }

    return FAILURE;
}


/*
 * This method writes first "len" bytes from "buffer" into "ssl-bio".
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
 * In this case, FAILURE must be returned immediately.
 */
static int secure_socket_write(SG_Socket* socket, unsigned char* buffer, int len)
{
    int rc = 0;

    rc = wolfSSL_write(socket->ssl, buffer, len);
    if(rc == len)
    {
        return SUCCESS;
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Bytes-Write failed in secure_socket_write, code = [%d]", rc);
        error_log(LOG_GLOBAL_BUFFER);
    }

    return FAILURE;
}


#define HANDLE_CATASTROPHIC_INIT_ERROR(obj, reset)                                                              \
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("\n\n%sFAILED IN [%s], we are doomed ..\n\n"), SSL_ERROR_STR, obj);    \
    error_log(LOG_GLOBAL_BUFFER);                                                                               \
                                                                                                                \
    if(reset == 1)                                                                                              \
    {                                                                                                           \
        resetDevice();                                                                                          \
    }                                                                                                           \
    else                                                                                                        \
    {                                                                                                           \
        return;                                                                                                 \
    }


WOLFSSL_CTX* solitary_ssl_ctx;

static unsigned ssl_init_try_once_done;
static unsigned char ssl_init_successful;

static void init_ssl()
{
    wolfSSL_Init();

    solitary_ssl_ctx = wolfSSL_CTX_new(wolfSSLv23_client_method());
    if(solitary_ssl_ctx == NULL)
    {
        HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("wolfSSL_CTX_new"), 1)
    }

    wolfSSL_CTX_set_verify(solitary_ssl_ctx, SSL_VERIFY_NONE, 0);

    ssl_init_successful = 1;
}
#endif


void fetch_gprs_params_from_sms(SG_Socket *socket)
{
#if GSM_INTERFACE_ENABLED == 1
    /* Empty-initialize the GSM-params. */
    memset(socket->gsmApn, 0, sizeof(socket->gsmApn));
    memset(socket->gsmUser, 0, sizeof(socket->gsmUser));
    memset(socket->gsmPass, 0, sizeof(socket->gsmPass));
    memset(socket->gsmPin, 0, sizeof(socket->gsmPin));
    memset(socket->provPin, 0, sizeof(socket->provPin));
	memset(socket->gsmMode, 0, sizeof(socket->gsmMode));

    if(smsFetched == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("\n\n SMS already fetched ... not re-fetching ...\n\n"));
        info_log(LOG_GLOBAL_BUFFER);

        goto sms_already_fetched;
    }

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
            sg_sprintf(LOG_GLOBAL_BUFFER, "\n\nProvisioning-Info SMS extracted from sim-area = [%s]\n\n", sms);
            info_log(LOG_GLOBAL_BUFFER);

            store_sms_in_config(sms, NULL, 0);
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

    smsFetched = 1;


sms_already_fetched:
    sg_sprintf(LOG_GLOBAL_BUFFER, "\n\nFinal (Converted) SMS being used for variables-extraction = [%s]\n\n", sms);
    info_log(LOG_GLOBAL_BUFFER);

    getJsonKeyValueIfPresent(sms, SG_APN, socket->gsmApn);
    getJsonKeyValueIfPresent(sms, SG_USER, socket->gsmUser);
    getJsonKeyValueIfPresent(sms, SG_PASS, socket->gsmPass);
    getJsonKeyValueIfPresent(sms, SG_PIN, socket->gsmPin);
    getJsonKeyValueIfPresent(sms, PROV_PIN, socket->provPin);
	getJsonKeyValueIfPresent(sms, SG_MODE, socket->gsmMode);

#if 0
    memset(socket->gsmApn, 0, sizeof(socket->gsmApn));
    strcpy(socket->gsmApn, "random_apn");
#endif

    sg_sprintf(LOG_GLOBAL_BUFFER,
               PROSTR("\n\nProvisioning-Params ::  sg_apn : [%s], sg_user : [%s], sg_pass : [%s], sg_pin : [%s], prov_pin : [%s], sg_mode : [%s]\n\n"),
               socket->gsmApn, socket->gsmUser, socket->gsmPass, socket->gsmPin, socket->provPin, socket->gsmMode);
    info_log(LOG_GLOBAL_BUFFER);

    startAndCountdownTimer(3, 0);
#endif
}

volatile unsigned char giveEnoughTimeBeforeRead;
void init_socket(SG_Socket *socket, const char *hostName, unsigned int port, const char *type, unsigned char secure)
{
    if(port == 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "port is 0 in init_socket.. bye ..");
        error_log(LOG_GLOBAL_BUFFER);

        resetDevice();
    }

    giveEnoughTimeBeforeRead = 0;


    socket->socketCorrupted = 1;

#if SSL_ENABLED == 1
    if(ssl_init_try_once_done == 0)
    {
        init_ssl();
        ssl_init_try_once_done = 1;
    }

    if(ssl_init_successful == 0)
    {
        return;
    }

#endif

    if((secure == 1) && (sslEnabledAtAppLayer == 1))
    {
#if SSL_ENABLED == 1
	    socket->read = secure_socket_read;
	    socket->write = secure_socket_write;
#else
        socket->read = socket_read;
        socket->write = socket_write;
#endif
    }
    else
    {
        socket->read = socket_read;
        socket->write = socket_write;
    }


    /* Keep a copy of connection-parameters, for easy book-keeping. */
    memset(socket->host, 0, sizeof(socket->host));
    sg_sprintf(socket->host, "%s", hostName);
    socket->port = port;
    socket->type = (char*) type;

#ifndef ETHERNET_AS_FIRST_PRIORITY
    fetch_gprs_params_from_sms(socket);
    watchdog_reset_and_enable(SOCKET_CONNECTION_SOLITARY_ATTEMPT_MAX_ALLOWED_TIME_SECONDS, "TRYING-SOCKET-CONNECTION-SOLITARY-ATTEMPT", 1);
#endif

    /* Connect the medium (socket). */
    connect_underlying_socket_medium_try_once(socket);

#ifndef ETHERNET_AS_FIRST_PRIORITY
    watchdog_disable(NULL, NULL);
#endif

#if SSL_ENABLED == 1
    if((secure == 1) && (sslEnabledAtAppLayer == 1))
    {
        if(socket->socketCorrupted == 0)
        {
			int rc = 0;

			socket->ssl = wolfSSL_new(solitary_ssl_ctx);
			if(socket->ssl == NULL)
			{
				HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("wolfSSL_new"), 1)
			}

			socket->ssl->IOCB_WriteCtx = socket;
			socket->ssl->IOCB_ReadCtx = socket;

            giveEnoughTimeBeforeRead = 1;

			rc = wolfSSL_connect(socket->ssl);
            if(rc != SSL_SUCCESS)
            {
				sg_sprintf(LOG_GLOBAL_BUFFER, "[wolfSSL_connect] failed with error-code [%d]", rc);
				error_log(LOG_GLOBAL_BUFFER);

                HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("wolfSSL_connect"), 1)
            }

            giveEnoughTimeBeforeRead = 0;
        }
    }
#endif
}


void release_socket(SG_Socket *socket)
{
    release_underlying_socket_medium_guaranteed(socket);

    sg_sprintf(LOG_GLOBAL_BUFFER,
               PROSTR("COMPLETE [TCP-SOCKET] STRUCTURE, INCLUDING THE UNDERLYING MEDIUM CLEANED FOR HOST = [%s], PORT = [%d]."),
               socket->host, socket->port);
    info_log(LOG_GLOBAL_BUFFER);
}


void get_ip_address_tokens(char *ipAddress, int *first, int *second, int *third, int *fourth)
{
	char small[5] = {0};

	get_nth_token_thread_safe(ipAddress, '.', 1, small, 1);
	*first = sg_atoi(small);

	memset(small, 0, sizeof(small));
	get_nth_token_thread_safe(ipAddress, '.', 2, small, 1);
	*second = sg_atoi(small);

	memset(small, 0, sizeof(small));
	get_nth_token_thread_safe(ipAddress, '.', 3, small, 1);
	*third = sg_atoi(small);

	memset(small, 0, sizeof(small));
	get_nth_token_thread_safe(ipAddress, '.', 4, small, 1);
	*fourth = sg_atoi(small);
}
