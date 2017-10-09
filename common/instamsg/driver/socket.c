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

#if SSL_ENABLED == 1
#include "./include/sg_openssl/ssl.h"
#include "./include/sg_openssl/bio.h"
#include "./include/sg_openssl/err.h"
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
SSL_CTX *solitary_ssl_ctx;

static unsigned char ssl_init_try_once_done;
static unsigned char ssl_init_successful;

static unsigned char wire_buffer[SSL_WIRE_BUFFER_SIZE];
static int WIRE_BUFFER_SIZE;


/*
 * Returns one of the following ::
 *
 *  SUCCESS   =>    if some bytes were sent over the network successfully.
 *  FAILURE   =>    otherwise.
 */
static int write_pending_data_to_network(SG_Socket* socket)
{
    int c = 0, rc = 0;

    c = BIO_pending(socket->network_bio);
    if(c > 0)
    {
        rc = BIO_read(socket->network_bio, wire_buffer, c);
        if(rc != c)
        {
            /*
             * Being bullet-proof in our checks always helps :)
             */
            sg_sprintf(LOG_GLOBAL_BUFFER, "%sExpected pending %d bytes to be read from network_bio, however %d bytes read",
                                          SSL_ERROR_STR, c, rc);

            error_log(LOG_GLOBAL_BUFFER);
            return FAILURE;
        }

#if 0
        sg_sprintf(LOG_GLOBAL_BUFFER, "Writing pending [%d] bytes", c);
        info_log(LOG_GLOBAL_BUFFER);
#endif

        /*
         * Remember, "socket_write" returns ::
         *
         *      SUCCESS, if bytes were sent successfully over the wire,
         *      FAILURE, otherwise.
         */
        return socket_write(socket, wire_buffer, c);
    }

    return SUCCESS;
}


/*
 * Returns one of the following ::
 *
 *  SUCCESS                 =>    if some bytes were read successfully from the wire successfully.
 *  SOCKET_READ_TIMEOUT     =>    if there is nothing available to be read from the wire.
 *  FAILURE                 =>    otherwise.
 */
static int read_pending_data_from_network(SG_Socket* socket, unsigned char must_read_some_bytes_from_wire)
{
    int c = 0, rc = 0;

    if(write_pending_data_to_network(socket) != SUCCESS)
    {
        return FAILURE;
    }

    c = BIO_get_read_request(socket->network_bio);
    if( (c == 0) && (must_read_some_bytes_from_wire == 0) )
    {
        /*
         * The network-bio is not in need of any bytes.
         *
         * But since we still landed on this method, is possible only if the application wants to read some
         * "potentially available" bytes. Any such "potential" bytes might actually have been pushed by the server,
         * or they may have been not.
         *
         * So, we just try to read 1 byte from the wire in a non-blocking way, and if we do get 1 byte, then we will push it into
         * the network-bio. Thereafter, the ssl-bio (via its network-bio pair) will command reading of some bytes in the next cycle.
         */
        c = 1;
    }

    if(c > 0)
    {
#if 0
        sg_sprintf(LOG_GLOBAL_BUFFER, "Reading pending [%d] bytes", c);
        info_log(LOG_GLOBAL_BUFFER);
#endif

        rc = socket_read(socket, wire_buffer, c, must_read_some_bytes_from_wire);
        if(rc == SUCCESS)
        {
            rc = BIO_write(socket->network_bio, wire_buffer, c);
            if(rc != c)
            {
                /*
                 * Being bullet-proof in our checks always helps :)
                 */
                sg_sprintf(LOG_GLOBAL_BUFFER, "%sExpected %d bytes to be written into network_bio, however %d bytes written",
                                              SSL_ERROR_STR, c, rc);

                error_log(LOG_GLOBAL_BUFFER);
                return FAILURE;
            }

            return SUCCESS;
        }
        else
        {
            return rc;  /* Covers both cases of FAILURE or SOCKET_READ_TIMEOUT being returned from lower layer */
        }
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "%sSome unknown case hit in \"read_pending_data_from_network\" .. debug the code ..", 
			                           SSL_ERROR_STR);
        error_log(LOG_GLOBAL_BUFFER);

        return FAILURE;
    }
}


#define HANDLE_NO_RETRY_CASE_IF_APPLICABLE(action)                                                                  \
    if(BIO_should_retry(socket->ssl_bio) == 0)                                                                      \
    {                                                                                                               \
        sg_sprintf(LOG_GLOBAL_BUFFER, "%sBIO_%s from ssl_bio failed with %d error-code", SSL_ERROR_STR, action, rc);\
        error_log(LOG_GLOBAL_BUFFER);                                                                               \
                                                                                                                    \
        return FAILURE;                                                                                             \
    }


#define HANDLE_SSL_NEED_FOR_WRITE_IF_APPLICABLE                                                                     \
    else if(BIO_should_write(socket->ssl_bio) != 0)                                                                 \
    {                                                                                                               \
        rc = write_pending_data_to_network(socket);                                                                 \
        if(rc == FAILURE)                                                                                           \
        {                                                                                                           \
            return FAILURE;                                                                                         \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            continue;                                                                                               \
        }                                                                                                           \
    }


#define HANDLE_UNEXPLORED_AREA_IF_APPLICABLE(action)                                                                \
    else                                                                                                            \
    {                                                                                                               \
        sg_sprintf(LOG_GLOBAL_BUFFER, "\n\n%sSSL-%s has reached unexplored territories.\n\n",                       \
                                       SSL_ERROR_STR, action);                                                      \
        error_log(LOG_GLOBAL_BUFFER);                                                                               \
                                                                                                                    \
        return FAILURE;                                                                                             \
    }


#define HANDLE_TWO_SSL_WRITE_READ_FROM_APP_CASES(action)                                                            \
    else if(rc == 0)                                                                                                \
    {                                                                                                               \
        sg_sprintf(LOG_GLOBAL_BUFFER, "%sBIO_%s to ssl_bio failed with %d error-code", SSL_ERROR_STR, action, rc);  \
        error_log(LOG_GLOBAL_BUFFER);                                                                               \
                                                                                                                    \
        return FAILURE;                                                                                             \
    }                                                                                                               \
                                                                                                                    \
    else if(rc != remaining_bytes)                                                                                  \
    {                                                                                                               \
        remaining_bytes = remaining_bytes - rc;                                                                     \
        continue;                                                                                                   \
    }


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
    int rc = FAILURE;
    int remaining_bytes = len;

    /*
     * We write the app-payload-bytes to the going-in side of SSL-BIO.
     */
    while(1)
    {
        rc = BIO_read(socket->ssl_bio, buffer + len - remaining_bytes, remaining_bytes);

        /* Case 1 */
        if(rc < 0)
        {
            HANDLE_NO_RETRY_CASE_IF_APPLICABLE("read")
            HANDLE_SSL_NEED_FOR_WRITE_IF_APPLICABLE
            else if(BIO_should_read(socket->ssl_bio) != 0)
            {
                if(1)
                {
                    /*
                     * Here, SSL_ERROR_WANT_READ might have been raised, as there may genuinely be no data to be read from socket.
                     * So, we are not sure whether we need to read anything from socket or not in actuality.
                     */
                    rc = read_pending_data_from_network(socket, 0);
                    if(rc == FAILURE)
                    {
                        /*
                         * Failure will never be tolerated ..
                         */
                        return FAILURE;
                    }
                    else if(rc == SOCKET_READ_TIMEOUT)
                    {
                        if(guaranteed == 1)
                        {
                            /*
                             * We must continue searching for bytes ...
                             */
                            continue;
                        }
                        else
                        {
                            /*
                             * Nothing available, so we must return the same to app-layer.
                             */
                            return SOCKET_READ_TIMEOUT;
                        }
                    }
                    else
                    {
                        /*
                         * We fulfilled ssl_bio's need to read ... retry ..
                         */
                        continue;
                    }
                }
            }
            HANDLE_UNEXPLORED_AREA_IF_APPLICABLE("read")
        }

        HANDLE_TWO_SSL_WRITE_READ_FROM_APP_CASES("read")

        else
        {
            return SUCCESS;
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
    int rc = FAILURE;
    int remaining_bytes = len;

    /*
     * We write the app-payload-bytes to the going-in side of SSL-BIO.
     */
    while(1)
    {
        rc = BIO_write(socket->ssl_bio, buffer + len - remaining_bytes, remaining_bytes);

        /* Case 1 */
        if(rc < 0)
        {
            HANDLE_NO_RETRY_CASE_IF_APPLICABLE("write")
            HANDLE_SSL_NEED_FOR_WRITE_IF_APPLICABLE
            else if(BIO_should_read(socket->ssl_bio) != 0)
            {
                if(1)
                {
                    /*
                     * Here, SSL wants to read some bytes from socket, before it can proceed.
                     * Obviously, this is a MUST-READ-FROM-SOCKET scenario.
                     */
                    rc = read_pending_data_from_network(socket, 1);
                    if(rc == FAILURE)
                    {
                        /*
                         * Failure will never be tolerated ..
                         */
                        return FAILURE;
                    }
                    else
                    {
                        /*
                         * We fulfilled ssl_bio's need to read ... retry ..
                         */
                        continue;
                    }
                }
            }
            HANDLE_UNEXPLORED_AREA_IF_APPLICABLE("write")
        }

        HANDLE_TWO_SSL_WRITE_READ_FROM_APP_CASES("write")

        else
        {
            /*
             * BIO_write to ssl-bio was successful.
             *
             * So, we send the encrypted-bytes (as emerging from network-bio) to network right-away.
             * In this way, we save an extra if-retry-then-write-pending-data cycle in the next iteration.
             */
            return write_pending_data_to_network(socket);
        }
    }
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


static void loadClientCertificateAndKeyIfPresent()
{
    char *buf = (char*) sg_malloc(2000);
    if(buf == NULL)
    {
        HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("Memory allocation in loadClientCertificateAndKeyIfPresent"), 1);
    }

    memset(buf, 0, sizeof(buf));
    load_client_certificate_into_buffer(buf, sizeof(buf));
    if(strlen(buf) > 0)
    {
        BIO *cert_bio = NULL;
        X509 *cert = NULL;

        cert_bio = BIO_new_mem_buf((void*)buf, -1);
        cert = PEM_read_bio_X509(cert_bio, NULL, 0, NULL);
        if(cert == NULL)
        {
            HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("PEM_read_bio_X509"), 1);
        }

        if(!SSL_CTX_use_certificate(solitary_ssl_ctx, cert))
        {
            HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("SSL_CTX_use_certificate"), 1)
        }
    }

    memset(buf, 0, sizeof(buf));
    load_client_private_key_into_buffer(buf, sizeof(buf));
    if(strlen(buf) > 0)
    {
        BIO *key_bio = NULL;
        RSA *rsa = NULL;

        key_bio = BIO_new_mem_buf((void*)buf, -1);
        rsa = PEM_read_bio_RSAPrivateKey(key_bio, NULL, 0, NULL);
        if(rsa == NULL)
        {
            HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("PEM_read_bio_RSAPrivateKey"), 1);
        }

        if(!SSL_CTX_use_RSAPrivateKey(solitary_ssl_ctx, rsa))
        {
            HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("SSL_CTX_use_RSAPrivateKey"), 1)
        }
    }

    sg_free(buf);
}


static void init_ssl()
{
    SSL_library_init();
    OpenSSL_add_ssl_algorithms();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ERR_load_crypto_strings();

    WIRE_BUFFER_SIZE = sizeof(wire_buffer);

    solitary_ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    if(solitary_ssl_ctx == NULL)
    {
        HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("SSL_CTX_new"), 1)
    }

    loadClientCertificateAndKeyIfPresent();
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


void init_socket(SG_Socket *socket, const char *hostName, unsigned int port, const char *type, unsigned char secure)
{
    if(port == 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "port is 0 in init_socket.. bye ..");
        error_log(LOG_GLOBAL_BUFFER);

        resetDevice();
    }


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

    socket->ssl = SSL_new(solitary_ssl_ctx);
    if(socket->ssl == NULL)
    {
        HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("SSL_new"), 0)
    }

    if (!BIO_new_bio_pair(&(socket->inter_bio), WIRE_BUFFER_SIZE, &(socket->network_bio), WIRE_BUFFER_SIZE))
    {
        HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("BIO_new_bio_pair"), 0)
    }

    socket->ssl_bio = BIO_new(BIO_f_ssl());
    if (!(socket->ssl_bio))
    {
        HANDLE_CATASTROPHIC_INIT_ERROR(PROSTR("BIO_new"), 0)
    }

    SSL_set_connect_state(socket->ssl);
    SSL_set_bio(socket->ssl, socket->inter_bio, socket->inter_bio);
    BIO_set_ssl(socket->ssl_bio, socket->ssl, BIO_NOCLOSE);
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
}


void release_socket(SG_Socket *socket)
{
    release_underlying_socket_medium_guaranteed(socket);

    sg_sprintf(LOG_GLOBAL_BUFFER,
               PROSTR("COMPLETE [TCP-SOCKET] STRUCTURE, INCLUDING THE UNDERLYING MEDIUM CLEANED FOR HOST = [%s], PORT = [%d]."),
               socket->host, socket->port);
    info_log(LOG_GLOBAL_BUFFER);
}
