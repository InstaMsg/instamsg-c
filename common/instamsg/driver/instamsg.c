/*******************************************************************************
 *
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *
 *
 *
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


#include "./include/instamsg.h"
#include "./include/httpclient.h"
#include "./include/json.h"
#include "./include/sg_mem.h"
#include "./include/sg_stdlib.h"
#include "./include/socket.h"
#include "./include/watchdog.h"
#include "./include/misc.h"
#include "./include/config.h"
#include "./include/data_logger.h"
#include "./include/globals.h"
#include "./include/upgrade.h"
#include "./include/gps.h"

#if FILE_SYSTEM_ENABLED == 1
#include "include/file_system.h"
#endif

#include <string.h>

InstaMsg instaMsg;
volatile unsigned char runBusinessLogicImmediately;

#define EMPTY_CLIENT_ID             PROSTR("EMPTY")
#define PROVISIONING_CLIENT_ID      PROSTR("PROVISIONING")

#define PROVISIONED                 PROSTR("PROVISIONED")
#define CONNECTED                   PROSTR("CONNECTED")
#define SIMULATED                   PROSTR("SIMULATED")


#define MAX_CYCLES_TO_WAIT_FOR_PINGRESP 10

int pubAckMsgId;
int pubAckRecvAttempts;
int pingRespAttempts;
unsigned char waitingForPuback;
char *lastPubTopic;
char *lastPubPayload;

enum PUBACK_STATE
{
    WAITING_FOR_PUBACK,
    NOT_WAITING_FOR_PUBACK,
    PUBACK_TIMEOUT
};

enum MESSAGE_SOURCE
{
    PERSISTENT_STORAGE,
    GENERAL
};

enum MESSAGE_SOURCE msgSource;
unsigned char rebootPending;

#define SECRET                                      PROSTR("SECRET")

#if MEDIA_STREAMING_ENABLED == 1
#include "./include/media.h"

static unsigned char mediaReplyReceived;
unsigned long mediaMessageRequestTime;
static int mediaReplyMessageWaitInterval;

static char streamId[MAX_BUFFER_SIZE];
#endif

#if HTTP_PROXY_ENABLED == 1
#include "./include/proxy.h"

static unsigned char proxyParamsReceived;
#endif

static unsigned char sendPacketIrrespective;
static unsigned char notifyServerOfSecretReceived;
static int actuallyEnsureGuaranteeWhereRequired;

#if FILE_SYSTEM_ENABLED == 1
static int autoUpgradeEnabled;
#endif

static void handleConnOrProvAckGeneric(InstaMsg *c, int connack_rc, const char *mode);

#if NTP_TIME_SYNC_PRESENT == 1
static unsigned char ntpPacket[48];
#endif

#if (NTP_TIME_SYNC_PRESENT == 1) || (GPS_TIME_SYNC_PRESENT == 1) || (GSM_TIME_SYNC_PRESENT == 1)
static DateParams dateParams;
#endif

#if (GPS_TIME_SYNC_PRESENT == 1) || (GSM_TIME_SYNC_PRESENT == 1)
static unsigned char gpsGsmTimeSyncFeatureEnabled;
#endif

volatile unsigned char timeSyncedViaExternalResources;

#define DEBUG_LOGGING_ENABLED                       PROSTR("DEBUG_LOGGING_ENABLED")

#if NTP_TIME_SYNC_PRESENT == 1
#define NTP_SERVER                                  PROSTR("NTP_SERVER")

static unsigned char ntpTimeSyncFeatureEnabled;
static char ntpServer[100];
#endif

#if (GPS_TIME_SYNC_PRESENT == 1) || (GSM_TIME_SYNC_PRESENT == 1)
#define MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC           PROSTR("MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC")

static char maxSecondsWaitForGpsGsmTimeSync[10];
#endif

#if SEND_GPS_LOCATION == 1
#define SEND_GPS_LOCATION_INTERVAL                  PROSTR("SEND_GPS_LOCATION_INTERVAL")

int sendGpsLocationInterval;
#endif

static volatile unsigned long timestampFromGSM;

#define DATA_LOG_TOPIC      PROSTR("topic")
#define DATA_LOG_PAYLOAD    PROSTR("payload")

static volatile unsigned char pingReqResponsePending;

static int statsDisplayInterval;
unsigned int bytes_sent_over_wire;
unsigned int bytes_received_over_wire;

static void publishAckReceived(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
}


static void serverLoggingTopicMessageArrived(InstaMsg *c, MQTTMessage *msg)
{
    /*
     * The payload is of the format ::
     *              {'client_id':'cc366750-e286-11e4-ace1-bc764e102b63','logging':1}
     */

    const char *CLIENT_ID = PROSTR("client_id");
    const char *LOGGING = PROSTR("logging");
    char *clientId, *logging;

    clientId = (char *)sg_malloc(MAX_BUFFER_SIZE);
    logging = (char *)sg_malloc(MAX_BUFFER_SIZE);
    if((clientId == NULL) || (logging == NULL))
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Could not allocate memory in serverLoggingTopicMessageArrived"));
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }
    memset(clientId, 0, MAX_BUFFER_SIZE);
    memset(logging, 0, MAX_BUFFER_SIZE);

    getJsonKeyValueIfPresent(msg->payload, CLIENT_ID, clientId);
    getJsonKeyValueIfPresent(msg->payload, LOGGING, logging);

    if( (strlen(clientId) > 0) && (strlen(logging) > 0) )
    {
        if(strcmp(logging, "1") == 0)
        {
            c->serverLoggingEnabled = 1;
            c->serverLogsStartTime = c->FRESH_SERVER_LOGS_TIME;

            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sEnabled."), SERVER_LOGGING);
            info_log(LOG_GLOBAL_BUFFER);
        }
        else
        {
            c->serverLoggingEnabled = 0;

            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sDisabled."), SERVER_LOGGING);
            info_log(LOG_GLOBAL_BUFFER);
        }
    }

exit:
    if(logging)
        sg_free(logging);

    if(clientId)
        sg_free(clientId);

    return;
}




static void publishQoS2CycleCompleted(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("PUBCOMP received for msg-id [%u]"), fixedHeaderPlusMsgId->msgId);
    info_log(LOG_GLOBAL_BUFFER);
}


static void NewMessageData(MessageData* md, InstaMsg *c, MQTTString* aTopicName, MQTTMessage* aMessgage) {
    md->topicName = aTopicName;
    md->message = aMessgage;
}


static int getNextPacketId(InstaMsg *c) {
    int id = c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
    return id;
}


static void attachResultHandler(InstaMsg *c, int msgId, unsigned int timeout, void (*resultHandler)(MQTTFixedHeaderPlusMsgId *))
{
    int i;

    if(resultHandler == NULL)
    {
        return;
    }


    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->resultHandlers[i].msgId == 0)
        {
            c->resultHandlers[i].msgId = msgId;
            c->resultHandlers[i].timeout = timeout;
            c->resultHandlers[i].fp = resultHandler;

            break;
        }
    }
}


static void freeLastPubMessageResources()
{
    if(lastPubPayload)
    {
        sg_free(lastPubPayload);
        lastPubPayload = NULL;
    }

    if(lastPubTopic)
    {
        sg_free(lastPubTopic);
        lastPubTopic = NULL;
    }
}


static void fireResultHandlerAndRemove(InstaMsg *c, MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    int i;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->resultHandlers[i].msgId == fixedHeaderPlusMsgId->msgId)
        {
            c->resultHandlers[i].fp(fixedHeaderPlusMsgId);
            c->resultHandlers[i].msgId = 0;

            if(fixedHeaderPlusMsgId->msgId == pubAckMsgId)
            {
                if(lastPubPayload != NULL)
                {
                    sg_sprintf(LOG_GLOBAL_BUFFER, "PUBACK received for message [%s]", lastPubPayload);
                    info_log(LOG_GLOBAL_BUFFER);
                }

                freeLastPubMessageResources();
                waitingForPuback = NOT_WAITING_FOR_PUBACK;
                pubAckRecvAttempts = 0;
            }

            break;
        }
    }
}


static void attachOneToOneHandler(InstaMsg *c,
                                  int msgId,
                                  unsigned int timeout,
                                  int (*oneToOneHandler)(OneToOneResult *))
{
    int i;

    if(oneToOneHandler == NULL)
    {
        return;
    }


    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->oneToOneHandlers[i].msgId == 0)
        {
            c->oneToOneHandlers[i].msgId = msgId;
            c->oneToOneHandlers[i].timeout = timeout;
            c->oneToOneHandlers[i].fp = oneToOneHandler;

            break;
        }
    }
}


static int fireOneToOneHandlerUsingMsgIdAsTheKey(InstaMsg *c, int msgId, OneToOneResult *oneToOneResult)
{
    int i;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->oneToOneHandlers[i].msgId == msgId)
        {
            c->oneToOneHandlers[i].fp(oneToOneResult);
            c->oneToOneHandlers[i].msgId = 0;

            return SUCCESS;
        }
    }

    return FAILURE;
}


static int doMqttSendPublish(int msgId,
                             int (*oneToOneHandler)(OneToOneResult *),
                             unsigned int timeout,
                             const char *topic, const char *message)
{
    attachOneToOneHandler(&instaMsg, msgId, timeout, oneToOneHandler);
    return publish(topic,
                   message,
                   QOS0,
                   0,
                   publishAckReceived,
                   MQTT_RESULT_HANDLER_TIMEOUT,
                   1);
}


static void MQTTReplyOneToOne(OneToOneResult *oneToOneResult,
                              const char *replyMessage,
                              int (*oneToOneHandler)(OneToOneResult *),
                              unsigned int oneToOneHandlerTimeout)
{
    InstaMsg *c = &instaMsg;
    int id = getNextPacketId(c);

    memset(messageBuffer, 0, sizeof(messageBuffer));
    sg_sprintf(messageBuffer,
               PROSTR("{\"message_id\": \"%u\", \"response_id\": \"%u\", \"reply_to\": \"%s\", \"body\": \"%s\", \"status\": 1}"),
               id,
               oneToOneResult->peerMsgId,
               c->clientIdComplete,
               replyMessage);

    doMqttSendPublish(id, oneToOneHandler, oneToOneHandlerTimeout, oneToOneResult->peerClientId, messageBuffer);
}


static void oneToOneMessageArrived(InstaMsg *c, MQTTMessage *msg)
{
    char *peerMessage = NULL, *peer = NULL;
    char peerMsgId[6];
    char responseMsgId[6];

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s Payload == [%s]"), ONE_TO_ONE, (char*) (msg->payload));
    info_log(LOG_GLOBAL_BUFFER);

    peerMessage = (char*) sg_malloc(MAX_BUFFER_SIZE);
    if(peerMessage == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not allocate memory for message received from peer"), ONE_TO_ONE);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }
    memset(peerMessage, 0, MAX_BUFFER_SIZE);
    getJsonKeyValueIfPresent(msg->payload, "body", peerMessage);

    peer = (char*) sg_malloc(50);
    if(peer == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not allocate memory for peer-value"), ONE_TO_ONE);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }
    memset(peer, 0, 50);
    getJsonKeyValueIfPresent(msg->payload, "reply_to", peer);

    memset(peerMsgId, 0, sizeof(peerMsgId));
    getJsonKeyValueIfPresent(msg->payload, "message_id", peerMsgId);

    memset(responseMsgId, 0, sizeof(responseMsgId));
    getJsonKeyValueIfPresent(msg->payload, "response_id", responseMsgId);

    if(strlen(peerMsgId) == 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sPeer-Message-Id not received ... not proceeding further"), ONE_TO_ONE);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }
    if(strlen(peer) == 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sPeer-value not received ... not proceeding further"), ONE_TO_ONE);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }


    {
        OneToOneResult oneToOneResult;

        oneToOneResult.succeeded = 1;
        oneToOneResult.peerMsg = peerMessage;
        oneToOneResult.peerClientId = peer;
        oneToOneResult.peerMsgId = sg_atoi(peerMsgId);
        oneToOneResult.reply = &MQTTReplyOneToOne;

        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Peer-Message = [%s], Peer = [%s], Peer-Message-Id = [%u]"),
                   oneToOneResult.peerMsg,
                   oneToOneResult.peerClientId,
                   oneToOneResult.peerMsgId);
        debug_log(LOG_GLOBAL_BUFFER);

        if(strlen(responseMsgId) == 0)
        {
            /*
             * This is a fresh message, so use the global callback.
             */
            if(c->oneToOneMessageHandler != NULL)
            {
                c->oneToOneMessageHandler(&oneToOneResult);
            }
        }
        else
        {
            /*
             * This is for an already exisiting message, that was sent by the current-client to the peer.
             * Call its handler (if at all it exists).
             */
            if(fireOneToOneHandlerUsingMsgIdAsTheKey(c, sg_atoi(responseMsgId), &oneToOneResult) == FAILURE)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sNo handler found for one-to-one for message-id [%s]"), ONE_TO_ONE, responseMsgId);
                error_log(LOG_GLOBAL_BUFFER);
            }
        }
    }

exit:
    if(peer)
        sg_free(peer);

    if(peerMessage)
        sg_free(peerMessage);
}


static void handleConfigReceived(InstaMsg *c, MQTTMessage *msg)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sReceived the config-payload [%s] from server"), CONFIG, (char*)(msg->payload));
    info_log(LOG_GLOBAL_BUFFER);

    process_config(msg->payload, 1);
}


#if (SSL_ENABLED == 1) || (SOCKET_SSL_ENABLED == 1)
static void saveClientAuthFieldInfoOntoDevice(char *payload, char *key, void (*func)(char *buffer))
{
    int sz = 2000;

    char *temp = (char*) sg_malloc(sz);
    if(temp == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory in saveClientAuthFieldInfoOntoDevice");
        error_log(LOG_GLOBAL_BUFFER);

        exitApp(0);
    }

    memset(temp, 0, sz);
    getJsonKeyValueIfPresent(payload, key, temp);
    if(strlen(temp) > 0)
    {
        char *slower = temp;
        char *faster = temp;

        while( (*faster) && (*(faster + 1) ))
        {
            if( (*faster == '\\') && (*(faster + 1) == 'n') )
            {
                *slower = '\n';
                faster = faster + 2;
            }
            else
            {
                *slower = *faster;
                faster = faster + 1;
            }

            slower = slower + 1;
        }

        *slower = 0;
        func(temp);
    }

    sg_free(temp);
}
#endif


static void processCertificateInfoIfAny(InstaMsg *c, char *payload)
{
#if (SSL_ENABLED == 1) || (SOCKET_SSL_ENABLED == 1)
    int sz = 2000;

    char *temp = (char*) sg_malloc(sz);
    if(temp == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory in processCertificateInfoIfAny");
        error_log(LOG_GLOBAL_BUFFER);

        exitApp(0);
    }

    memset(temp, 0, sz);
    getJsonKeyValueIfPresent(payload, "secure_ssl_certificate", temp);
    if(strcmp(temp, "true") == 0)
    {
        sg_free(temp);

        saveClientAuthFieldInfoOntoDevice(payload, "certificate", save_client_certificate_from_buffer);
        saveClientAuthFieldInfoOntoDevice(payload, "key", save_client_private_key_from_buffer);
    }
#endif
}


static void handleCertReceived(InstaMsg *c, MQTTMessage *msg)
{
    processCertificateInfoIfAny(c, (char*)(msg->payload));

    sg_sprintf(LOG_GLOBAL_BUFFER, "Rebooting machine, as certificate has been updated.");
    info_log(LOG_GLOBAL_BUFFER);

    exitApp(0);
}


static int sendPacket(InstaMsg * c, SG_Socket *s, unsigned char *buf, int length)
{
    int rc = SUCCESS;

    /*
     * We assume that if a packet cannot be sent within 30 seconds,
     * there has been some (undetectable) issue somewehre.
     */
    watchdog_reset_and_enable(60, "sendPacket", 1);

    if(s->socketCorrupted == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Socket not available at physical layer .. so packet cannot be sent to server."));
        error_log(LOG_GLOBAL_BUFFER);

        rc = FAILURE;
        goto exit;
    }

    if(sendPacketIrrespective != 1)
    {
        if(c->connected == 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("No CONNACK received from server .. so packet cannot be sent to server."));
            error_log(LOG_GLOBAL_BUFFER);

            rc = FAILURE;
            goto exit;
        }
    }
    sendPacketIrrespective = 0;

    if(s->write(s, buf, length) == FAILURE)
    {
        s->socketCorrupted = 1;
        rc = FAILURE;
    }

exit:
    watchdog_disable(NULL, NULL);
    return rc;
}



static int readPacket(InstaMsg* c, MQTTFixedHeader *fixedHeader)
{
    MQTTHeader header = {0};
    int rc = FAILURE;
    int len = 0;
    int rem_len = 0;
    unsigned char i;
    int multiplier = 1;
    int numRetries = MAX_TRIES_ALLOWED_WHILE_READING_FROM_SOCKET_MEDIUM;

    watchdog_reset_and_enable(60 * MAX_TRIES_ALLOWED_WHILE_READING_FROM_SOCKET_MEDIUM * SOCKET_READ_TIMEOUT_SECS,
                              "readPacket", 1);

    if((c->ipstack).socketCorrupted == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Socket not available at physical layer .. so packet cannot be read from server."));
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }


    /*
     * 0. Before reading the packet, memset the read-buffer to all-empty, else there will be issues
     *    processing the buffer as a string.
     */
    memset((char*)c->readbuf, 0, sizeof(c->readbuf));


    /*
     * 1. read the header byte.  This has the packet type in it.
     */
    do
    {
        rc = (c->ipstack).read(&(c->ipstack), c->readbuf, 1, 0);
        if(rc == FAILURE)
        {
            (c->ipstack).socketCorrupted = 1;
            goto exit;
        }

        if(rc == SOCKET_READ_TIMEOUT)
        {
            numRetries--;
        }
    } while((rc == SOCKET_READ_TIMEOUT) && (numRetries > 0));

    /*
     * If at this point, we still had a socket-timeout, it means we really have nothing to read.
     */
    if(rc == SOCKET_READ_TIMEOUT)
    {
        goto exit;
    }

    len = 1;
    /* 2. read the remaining length.  This is variable in itself
     */

    rem_len = 0;
    do
    {
        if((c->ipstack).read(&(c->ipstack), &i, 1, 1) == FAILURE) /* Pseudo-Blocking Call */
        {
            (c->ipstack).socketCorrupted = 1;
            goto exit;
        }

        rem_len += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);


    len += MQTTPacket_encode(c->readbuf + 1, rem_len);

    /* 3. read the rest of the buffer */
    if(rem_len > 0)
    {
        if((c->ipstack).read(&(c->ipstack), c->readbuf + len, rem_len, 1) == FAILURE) /* Pseudo-Blocking Call */
        {
            (c->ipstack).socketCorrupted = 1;
            goto exit;
        }
    }

    header.byte = c->readbuf[0];
    fillFixedHeaderFieldsFromPacketHeader(fixedHeader, &header);

    rc = SUCCESS;

exit:
    watchdog_disable(NULL, NULL);
    return rc;
}


/*
 * assume topic filter and name is in correct format
 * # can only be at end
 * + and # can only be next to separator
 */
static char isTopicMatched(char* topicFilter, MQTTString* topicName)
{
    char* curf = topicFilter;
    char* curn = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;

    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   /* skip until we meet the next separator, or end of string */
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    /* skip until end of string */
        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}


static int deliverMessageToSelf(InstaMsg* c, MQTTString* topicName, MQTTMessage* message)
{
    int i;
    int rc = FAILURE;
    enum QoS qos;

    /* we have to find the right message handler - indexed by topic */
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[i].topicFilter) ||
                isTopicMatched((char*)c->messageHandlers[i].topicFilter, topicName)))
        {
            if (c->messageHandlers[i].fp != NULL)
            {
                MessageData md;
                NewMessageData(&md, c, topicName, message);
                c->messageHandlers[i].fp(&md);
                rc = SUCCESS;
            }
        }
    }

    if (rc == FAILURE && c->defaultMessageHandler != NULL)
    {
        MessageData md;
        NewMessageData(&md, c, topicName, message);
        c->defaultMessageHandler(&md);
        rc = SUCCESS;
    }

    /*
     * Send the ACK to the server too, if applicable
     */
    qos = (message->fixedHeaderPlusMsgId).fixedHeader.qos;
    if (qos != QOS0)
    {
        int len = 0;

        RESET_GLOBAL_BUFFER;

        if (qos == QOS1)
        {
            len = MQTTSerialize_ack(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), PUBACK, 0, (message->fixedHeaderPlusMsgId).msgId);
        }
        else if (qos == QOS2)
        {
            len = MQTTSerialize_ack(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), PUBREC, 0, (message->fixedHeaderPlusMsgId).msgId);
        }

        if (len > 0)
        {
            rc = sendPacket(c, &(c->ipstack), GLOBAL_BUFFER, len);
        }
    }

    return rc;
}


static int fireResultHandlerUsingMsgIdAsTheKey(InstaMsg *c)
{
    int msgId = -1;

    MQTTFixedHeaderPlusMsgId fixedHeaderPlusMsgId;
    if (MQTTDeserialize_FixedHeaderAndMsgId(&fixedHeaderPlusMsgId, c->readbuf, sizeof(c->readbuf)) == SUCCESS)
    {
        msgId = fixedHeaderPlusMsgId.msgId;
        fireResultHandlerAndRemove(c, &fixedHeaderPlusMsgId);
    }

    return msgId;
}


static void logJsonFailureMessageAndReturn(const char *module, const char *key, MQTTMessage *msg)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not find key [%s] in message-payload [%s] .. not proceeding further"),
               module, key, (char*) (msg->payload));
    error_log(LOG_GLOBAL_BUFFER);
}


#if MEDIA_STREAMING_ENABLED == 1
static void handleMediaStopMessage(InstaMsg *c)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sStopping ....."), MEDIA);
    info_log(LOG_GLOBAL_BUFFER);

    stop_streaming();

    RESET_GLOBAL_BUFFER;
    sg_sprintf((char*)GLOBAL_BUFFER, PROSTR("{'to':'%s','from':'%s','type':3,'stream_id': '%s'}"),
               c->clientIdComplete, c->clientIdComplete, streamId);

    publish(c->mediaTopic,
            (char*)GLOBAL_BUFFER,
            QOS0,
            0,
            NULL,
            MQTT_RESULT_HANDLER_TIMEOUT,
            1);
}


static void handleMediaPauseMessage(InstaMsg *c)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sPausing ....."), MEDIA);
    info_log(LOG_GLOBAL_BUFFER);

    pause_streaming();
}


static void broadcastMedia(InstaMsg * c, char *sdpAnswer)
{
    {
        /*
         * We hard-code the media-server-ip-address.
         */
        memset(c->mediaServerIpAddress, 0, sizeof(c->mediaServerIpAddress));
        strcpy(c->mediaServerIpAddress, "23.253.42.123");
    }

    {
        /*
         * We need to extract port from the string of type
         *
         *              m=video 12345 RTP/AVP 96
         */
        const char *stringToSearch = "m=video ";

        char *pos = strstr(sdpAnswer, stringToSearch);
        if(pos != NULL)
        {
            char *token = strtok(pos, " ");

            if(token != NULL)
            {
                /*
                * Find the server-port.
                */
                memset(c->mediaServerPort, 0, sizeof(c->mediaServerPort));
                strcpy(c->mediaServerPort, strtok(NULL, " "));

                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sMedia-Server IP-Address and Port being used for streaming [%s], [%s]"),
                                              MEDIA, c->mediaServerIpAddress,  c->mediaServerPort);
                info_log(LOG_GLOBAL_BUFFER);

                mediaReplyReceived = 1;
                create_and_start_streaming_pipeline(c->mediaServerIpAddress, c->mediaServerPort);

                return;
            }
        }
    }

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not find server-port for streaming.. not doing anything else !!!"), MEDIA);
    error_log(LOG_GLOBAL_BUFFER);
}


static void handleMediaReplyMessage(InstaMsg *c, MQTTMessage *msg)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sReceived media-reply-message [%s]"), MEDIA, (char*)msg->payload);
    info_log(LOG_GLOBAL_BUFFER);

    {
        const char *STREAM_ID = PROSTR("stream_id");
        const char *SDP_ANSWER = PROSTR("sdp_answer");

        char *sdpAnswer;

        memset(streamId, 0, sizeof(streamId));
        getJsonKeyValueIfPresent(msg->payload, STREAM_ID, streamId);

        sdpAnswer = (char *)sg_malloc(MAX_BUFFER_SIZE);
        if(sdpAnswer == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not allocate memory for sdp-answer", MEDIA);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }
        memset(sdpAnswer, 0, MAX_BUFFER_SIZE);
        getJsonKeyValueIfPresent(msg->payload, SDP_ANSWER, sdpAnswer);

        if(strlen(sdpAnswer) > 0)
        {
            broadcastMedia(c, sdpAnswer);
        }
        else
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not process sdp-answer ... media will not start streaming !!!"), MEDIA);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

exit:
        if(sdpAnswer)
        {
            sg_free(sdpAnswer);
        }
    }
}


static void handleMediaStreamsMessage(InstaMsg *c, MQTTMessage *msg)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sReceived media-streams-message [%s]"), MEDIA, (char*) msg->payload);
    info_log(LOG_GLOBAL_BUFFER);

    {
        const char *REPLY_TO = PROSTR("reply_to");
        const char *MESSAGE_ID = PROSTR("message_id");
        const char *METHOD = PROSTR("method");

        char *replyTopic, *messageId, *method;
        replyTopic = (char*) sg_malloc(100);
        messageId = (char*) sg_malloc(MAX_BUFFER_SIZE);
        method = (char*) sg_malloc(10);

        if((replyTopic == NULL) || (messageId == NULL) || (method == NULL))
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not allocate memory for replyTopic/messageId/method", MEDIA);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

        getJsonKeyValueIfPresent(msg->payload, REPLY_TO, replyTopic);
        getJsonKeyValueIfPresent(msg->payload, MESSAGE_ID, messageId);
        getJsonKeyValueIfPresent(msg->payload, METHOD, method);

        if(strlen(replyTopic) == 0)
        {
            logJsonFailureMessageAndReturn(MEDIA, REPLY_TO, msg);
            goto exit;
        }

        if(strlen(messageId) == 0)
        {
            logJsonFailureMessageAndReturn(MEDIA, MESSAGE_ID, msg);
            goto exit;
        }

        if(strlen(method) == 0)
        {
            logJsonFailureMessageAndReturn(MEDIA, METHOD, msg);
            goto exit;
        }

        if(strcmp(method, "GET") == 0)
        {
            RESET_GLOBAL_BUFFER;
            sg_sprintf((char*) GLOBAL_BUFFER, PROSTR("{\"response_id\": \"%s\", \"status\": 1, \"streams\": \"[%s]\"}"), messageId, streamId);

            publish(replyTopic,
                    (char*)GLOBAL_BUFFER,
                    QOS0,
                    0,
                    NULL,
                    MQTT_RESULT_HANDLER_TIMEOUT,
                    1);
        }

exit:
        if(method)
            sg_free(method);

        if(messageId)
            sg_free(messageId);

        if(replyTopic)
            sg_free(replyTopic);
    }
}


static void initiateStreaming()
{
    InstaMsg *c = &instaMsg;

    memset(c->selfIpAddress, 0, sizeof(c->selfIpAddress));
    get_device_ip_address(c->selfIpAddress, sizeof(c->selfIpAddress));

    memset(messageBuffer, 0, sizeof(messageBuffer));
    sg_sprintf(messageBuffer,
               "{"
                    "'to': '%s', "
                    "'sdp_offer' : 'v=0\r\n"
                                   "o=- 0 0 IN IP4 %s\r\n"
                                   "s=\r\n"
                                   "c=IN IP4 %s\r\n"
                                   "t=0 0\r\n"
                                   "a=charset:UTF-8\n"
                                   "a=recvonly\r\n"
                                   "m=video 50004 RTP/AVP 96\r\n"
                                   "a=rtpmap:96 H264/90000\r\n"
                                    "', "
	    			"'from': '%s', "
                    "'protocol' : 'rtp', "
                    "'type':'7', "
                    "'stream_id':'%s', "
	    			"'record': True"
	    		"}",
                c->clientIdComplete,
                                    c->selfIpAddress,
                                    c->selfIpAddress,
                c->clientIdComplete,
                c->clientIdComplete);


    mediaReplyReceived = 2;
    mediaMessageRequestTime = getCurrentTick();

    publish(c->mediaTopic,
	    	messageBuffer,
			QOS0,
			0,
			NULL,
			MQTT_RESULT_HANDLER_TIMEOUT,
			1);
}
#endif


#if HTTP_PROXY_ENABLED == 1
static void handleProxyMessage(InstaMsg *c, MQTTMessage *msg)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sReceived proxy-message [%s]"), PROXY, (char*)msg->payload);
    info_log(LOG_GLOBAL_BUFFER);

    memset(c->proxyUser, 0, sizeof(c->proxyUser));
    getJsonKeyValueIfPresent((char*)msg->payload, "user", c->proxyUser);

    memset(c->proxyPasswd, 0, sizeof(c->proxyPasswd));
    getJsonKeyValueIfPresent((char*)msg->payload, "passwd", c->proxyPasswd);

    memset(c->proxyServer, 0, sizeof(c->proxyServer));
    getJsonKeyValueIfPresent((char*)msg->payload, "server", c->proxyServer);

    memset(c->proxyPort, 0, sizeof(c->proxyPort));
    getJsonKeyValueIfPresent((char*)msg->payload, "port", c->proxyPort);


    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sProxy-Parameters :: User = [%s], Passwd = [%s], Server = [%s], Port = [%s]"),
               PROXY, c->proxyUser, c->proxyPasswd, c->proxyServer, c->proxyPort);
    info_log(LOG_GLOBAL_BUFFER);

    proxyParamsReceived = 1;
}
#endif


static void handleFileTransfer(InstaMsg *c, MQTTMessage *msg)
{
    const char *REPLY_TOPIC = PROSTR("reply_to");
    const char *MESSAGE_ID = PROSTR("message_id");
    const char *METHOD = PROSTR("method");
    char *replyTopic, *messageId, *method, *url, *filename, *ackMessage;
    unsigned char downloadCase = 0;

    replyTopic = (char *)sg_malloc(MAX_BUFFER_SIZE);
    memset(replyTopic, 0, MAX_BUFFER_SIZE);

    messageId = (char *)sg_malloc(50);
    memset(messageId, 0, 50);

    method = (char *)sg_malloc(20);
    memset(method, 0, 20);

    url = (char *)sg_malloc(MAX_BUFFER_SIZE);
    memset(url, 0, MAX_BUFFER_SIZE);

    filename = (char *)sg_malloc(MAX_BUFFER_SIZE);
    memset(filename, 0, MAX_BUFFER_SIZE);

    ackMessage = (char *)sg_malloc(MAX_BUFFER_SIZE);
    memset(ackMessage, 0, MAX_BUFFER_SIZE);

    getJsonKeyValueIfPresent(msg->payload, REPLY_TOPIC, replyTopic);
    getJsonKeyValueIfPresent(msg->payload, MESSAGE_ID, messageId);
    getJsonKeyValueIfPresent(msg->payload, METHOD, method);
    getJsonKeyValueIfPresent(msg->payload, "url", url);
    getJsonKeyValueIfPresent(msg->payload, "filename", filename);


    if(strlen(replyTopic) == 0)
    {
        logJsonFailureMessageAndReturn(FILE_TRANSFER, REPLY_TOPIC, msg);
        goto exit;
    }
    if(strlen(messageId) == 0)
    {
        logJsonFailureMessageAndReturn(FILE_TRANSFER, MESSAGE_ID, msg);
        goto exit;
    }
    if(strlen(method) == 0)
    {
        logJsonFailureMessageAndReturn(FILE_TRANSFER, METHOD, msg);
        goto exit;
    }



    if( (   (strcmp(method, "POST") == 0) || (strcmp(method, "PUT") == 0)   ) &&
            (strlen(filename) > 0) &&
            (strlen(url) > 0)   )
    {
        downloadCase = 1;
    }

    if(downloadCase == 1)
    {
        /*
         * Remove the protocol:hostname.
         *
         * Eg. ::  https://localhost, http://localhost, etc.
         */
        if(strlen(url) > 0)
        {
            char *subs = "//";

            char *tmp = strstr(url, subs);
            if(tmp != NULL)
            {
                tmp = strstr(tmp + strlen(subs), "/");
                if(tmp != NULL)
                {
                    strncpy(url, tmp, strlen(tmp));
                    url[strlen(tmp)] = 0;
                }
            }
        }
    }


    if(downloadCase == 1)
    {
        int ackStatus = 0;

        /*
         * Behaviour of File-Download Status-Notification to user
         * (as per the scenario tested, when a browser-client uploads file, and a C-client downloads the file).
         * ====================================================================================================
         *
         * While browser-client uploads the file to server, "Uploading %" is shown.
         *
         * Once the upload is complete, the C-client starts downloading, and the browser-client sees a "Waiting .."
         * note .. (in the browser-lower panel).
         *
         * Now, following scenarios arise ::
         *
         * a)
         * C-client finishes the downloading, returns status 200 and the ACK-message is sent to server
         * with status 1.
         *
         * In this case, the "Waiting .." message disappears (as expected), and an additional ioEYE-message
         * "File uploaded successfully" is displayed to browser-client.
         *
         *
         * b)
         * C-client might or might not finish downloading, but it returns a status other than 200, and the ACK-message
         * is sent to server with status 0.
         *
         * In this case, the "Waiting .." message disappears (as expected), but no additional ioEYE message is displayed.
         * (MAY BE, SOME ERROR-NOTIFICATION SHOULD BE SHOWN TO THE BROWSER-CLIENT).
         *
         *
         * c)
         * C-client might or might not finish downloading, but no ACK-message is sent to the server whatsoever.
         *
         * In this case, the "Waiting .." message is kept showing on the browser-client (posssibly timing out after
         * a long time).
         *
         *
         * ALL IN ALL, IF THE "Waiting .." MESSAGE DISAPPEARS, AND THE "File uploaded succcessfully" MESSAGE IS SEEN,
         * IT MEANS THE FILE-TRANSFER COMPLETED, AND THAT TOO PERFECTLY SUCCESSFULLY.
         *
         */

        RESET_HTTP_RESPONSE;
        downloadFile(url, filename, NULL, NULL, 10, &httpResponse);

        if(httpResponse.status == HTTP_FILE_DOWNLOAD_SUCCESS)
        {
            ackStatus = 1;
        }
        sg_sprintf(ackMessage, PROSTR("{\"response_id\": \"%s\", \"status\": %d}"), messageId, ackStatus);

    }
    else if( (strcmp(method, "GET") == 0) && (strlen(filename) == 0))
    {
        char *fileListing;

        RESET_GLOBAL_BUFFER;
        fileListing = (char*)GLOBAL_BUFFER;

#if FILE_SYSTEM_ENABLED == 1
        (c->singletonUtilityFs).getFileListing(&(c->singletonUtilityFs), fileListing, MAX_BUFFER_SIZE, ".");
#else
        strcat(fileListing, "{}");
#endif

        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s: [%s]"), FILE_LISTING, fileListing);
        info_log(LOG_GLOBAL_BUFFER);

        sg_sprintf(ackMessage, PROSTR("{\"response_id\": \"%s\", \"status\": 1, \"files\": %s}"), messageId, fileListing);
    }
    else if( (strcmp(method, "DELETE") == 0) && (strlen(filename) > 0))
    {
        int status = FAILURE;

#if FILE_SYSTEM_ENABLED == 1
        status = (c->singletonUtilityFs).deleteFile(&(c->singletonUtilityFs), filename);
#endif

        if(status == SUCCESS)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s[%s] deleted successfully."), FILE_DELETE, filename);
            info_log(LOG_GLOBAL_BUFFER);

            sg_sprintf(ackMessage, PROSTR("{\"response_id\": \"%s\", \"status\": 1}"), messageId);
        }
        else
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s[%s] could not be deleted :("), FILE_DELETE, filename);
            error_log(LOG_GLOBAL_BUFFER);

            sg_sprintf(ackMessage, PROSTR("{\"response_id\": \"%s\", \"status\": 0, \"error_msg\":\"%s\"}"),
                       messageId, PROSTR("File-Removal Failed :("));
        }
    }
    else if( (strcmp(method, "GET") == 0) && (strlen(filename) > 0))
    {
#if FILE_SYSTEM_ENABLED == 1
        HTTPResponse response = {0};

        char *clientIdBuf;
        KeyValuePairs headers[5];

        headers[0].key = "Authorization";
        headers[0].value = c->connectOptions.password.cstring;

        headers[1].key = "ClientId";

        clientIdBuf = (char*) sg_malloc(MAX_BUFFER_SIZE);
        if(clientIdBuf == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFailed to allocate memory"), FILE_UPLOAD);
            error_log(LOG_GLOBAL_BUFFER);

            goto terminateFileUpload;
        }
        memset(clientIdBuf, 0, MAX_BUFFER_SIZE);

        sg_sprintf(clientIdBuf, "%s", c->clientIdComplete);
        headers[1].value = clientIdBuf;

        headers[2].key = "Content-Type";
        headers[2].value = "multipart/form-data; boundary=" POST_BOUNDARY;

        headers[3].key = CONTENT_LENGTH;
        headers[3].value = "0"; /* This will be updated to proper bytes later. */

        headers[4].key = 0;
        headers[4].value = 0;


        RESET_HTTP_RESPONSE;
        uploadFile(c->fileUploadUrl, filename, NULL, headers, 10, &httpResponse);

terminateFileUpload:

        if(clientIdBuf)
            sg_free(clientIdBuf);
        if(response.status == HTTP_FILE_UPLOAD_SUCCESS)
        {
            sg_sprintf(ackMessage, PROSTR("{\"response_id\": \"%s\", \"status\": 1, \"url\": \"%s\"}"), messageId, response.body);
        }
        else
        {
            sg_sprintf(ackMessage, PROSTR("{\"response_id\": \"%s\", \"status\": 0}"), messageId);
        }
#else
        sg_sprintf(ackMessage, PROSTR("{\"response_id\": \"%s\", \"status\": 0}"), messageId);
#endif
    }


    /*
     * Send the acknowledgement, along with the ackStatus (success/failure).
     */
    publish(replyTopic,
            ackMessage,
            (msg->fixedHeaderPlusMsgId).fixedHeader.qos,
            (msg->fixedHeaderPlusMsgId).fixedHeader.dup,
            NULL,
            MQTT_RESULT_HANDLER_TIMEOUT,
            1);

exit:
    if(ackMessage)
        sg_free(ackMessage);

    if(filename)
        sg_free(filename);

    if(url)
        sg_free(url);

    if(method)
        sg_free(method);

    if(messageId)
        sg_free(messageId);

    if(replyTopic)
        sg_free(replyTopic);


    return;
}


static void checkAndRemoveExpiredHandler(int *msgId, unsigned int *timeout, const char *info)
{
    if(*msgId == 0)
    {
        return;
    }

    if(*timeout <= 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("No %s received for msgid [%u], removing.."), info, *msgId);
        info_log(LOG_GLOBAL_BUFFER);

        *msgId = 0;
    }
    else
    {
        *timeout = *timeout - 1;
    }
}


static void removeExpiredResultHandlers(InstaMsg *c)
{
    int i;
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        checkAndRemoveExpiredHandler(&(c->resultHandlers[i].msgId), &(c->resultHandlers[i].timeout), "pub/sub response");
    }
}

static void removeExpiredOneToOneResponseHandlers(InstaMsg *c)
{
    int i;
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        checkAndRemoveExpiredHandler(&(c->oneToOneHandlers[i].msgId), &(c->oneToOneHandlers[i].timeout), "one-to-one response");
    }
}


static void waitForPingResp();
void sendPingReqToServer(InstaMsg *c)
{
    int len;

    RESET_GLOBAL_BUFFER;
    len = MQTTSerialize_pingreq(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));

    if((c->ipstack).socketCorrupted == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER,
                   PROSTR("Socket not available at physical layer .. so server cannot be pinged for maintaining keep-alive."));
        error_log(LOG_GLOBAL_BUFFER);

        return;
    }

    if (len > 0)
    {
        pingReqResponsePending = 1;
        sendPacket(c, &(c->ipstack), GLOBAL_BUFFER, len);

        waitForPingResp();
    }
}


void clearInstaMsg(InstaMsg *c)
{
    release_socket(&(c->ipstack));

#if FILE_SYSTEM_ENABLED == 1
    release_file_system(&(c->singletonUtilityFs));
#endif

    c->connected = 0;

#if MEDIA_STREAMING_ENABLED == 1
    /*
     * If we need to clear (most probably due to no network-connection, we must then restart if media-streaming is enabled.
     * Else, the media-reply message will not be re-sent.
     */
    exitApp(0);
#endif
}


static void setValuesOfSpecialTopics(InstaMsg *c)
{
    memset(c->filesTopic, 0, sizeof(c->filesTopic));
    sg_sprintf(c->filesTopic, PROSTR("instamsg/clients/%s/files"), c->clientIdComplete);

    memset(c->rebootTopic, 0, sizeof(c->rebootTopic));
    sg_sprintf(c->rebootTopic, PROSTR("instamsg/clients/%s/reboot"), c->clientIdComplete);

    memset(c->enableServerLoggingTopic, 0, sizeof(c->enableServerLoggingTopic));
    sg_sprintf(c->enableServerLoggingTopic, PROSTR("instamsg/clients/%s/enableServerLogging"), c->clientIdComplete);

    memset(c->updateCertTopic, 0, sizeof(c->updateCertTopic));
    sg_sprintf(c->updateCertTopic, PROSTR("instamsg/clients/%s/updateCert"), c->clientIdComplete);

    memset(c->serverLogsTopic, 0, sizeof(c->serverLogsTopic));
    sg_sprintf(c->serverLogsTopic, PROSTR("instamsg/clients/%s/logs"), c->clientIdComplete);

    memset(c->fileUploadUrl, 0, sizeof(c->fileUploadUrl));
    sg_sprintf(c->fileUploadUrl, PROSTR("/api/beta/clients/%s/files"), c->clientIdComplete);

    memset(c->receiveConfigTopic, 0, sizeof(c->receiveConfigTopic));
    sg_sprintf(c->receiveConfigTopic, PROSTR("instamsg/clients/%s/config/serverToClient"), c->clientIdComplete);

#if MEDIA_STREAMING_ENABLED == 1
    memset(c->mediaTopic, 0, sizeof(c->mediaTopic));
    sg_sprintf(c->mediaTopic, PROSTR("instamsg/clients/%s/media"), c->clientIdComplete);

    memset(c->mediaReplyTopic, 0, sizeof(c->mediaReplyTopic));
    sg_sprintf(c->mediaReplyTopic, PROSTR("instamsg/clients/%s/mediareply"), c->clientIdComplete);

    memset(c->mediaStopTopic, 0, sizeof(c->mediaStopTopic));
    sg_sprintf(c->mediaStopTopic, PROSTR("instamsg/clients/%s/mediastop"), c->clientIdComplete);

    memset(c->mediaPauseTopic, 0, sizeof(c->mediaPauseTopic));
    sg_sprintf(c->mediaPauseTopic, PROSTR("instamsg/clients/%s/mediapause"), c->clientIdComplete);

    memset(c->mediaStreamsTopic, 0, sizeof(c->mediaStreamsTopic));
    sg_sprintf(c->mediaStreamsTopic, PROSTR("instamsg/clients/%s/mediastreams"), c->clientIdComplete);
#endif

#if HTTP_PROXY_ENABLED == 1
    memset(c->proxyTopic, 0, sizeof(c->proxyTopic));
    sg_sprintf(c->proxyTopic, PROSTR("instamsg/clients/%s/proxy"), c->clientIdComplete);
#endif

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("\n\nThe special-topics value :: \n\n"
             "\r\nFILES_TOPIC = [%s],"
             "\r\nREBOOT_TOPIC = [%s],"
             "\r\nENABLE_SERVER_LOGGING_TOPIC = [%s],"
             "\r\nSERVER_LOGS_TOPIC = [%s],"
             "\r\nFILE_UPLOAD_URL = [%s],"
             "\r\nCONFIG_FROM_SERVER_TO_CLIENT = [%s],"
             "\r\nUPDATE_CERT_TOPIC = [%s]"),
              c->filesTopic, c->rebootTopic, c->enableServerLoggingTopic,
              c->serverLogsTopic, c->fileUploadUrl, c->receiveConfigTopic, c->updateCertTopic);
    info_log(LOG_GLOBAL_BUFFER);

#if MEDIA_STREAMING_ENABLED == 1
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("\r\nMEDIA_TOPIC = [%s],"
             "\r\nMEDIA_REPLY_TOPIC = [%s],"
             "\r\nMEDIA_STOP_TOPIC = [%s],"
             "\r\nMEDIA_PAUSE_TOPIC = [%s],"
             "\r\nMEDIA_STREAMS_TOPIC = [%s]"),
             c->mediaTopic, c->mediaReplyTopic, c->mediaStopTopic, c->mediaPauseTopic, c->mediaStreamsTopic);
    info_log(LOG_GLOBAL_BUFFER);
#endif

#if HTTP_PROXY_ENABLED == 1
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("\r\nPROXY_TOPIC = [%s]"), c->proxyTopic);
    info_log(LOG_GLOBAL_BUFFER);
#endif

}


#if (GPS_TIME_SYNC_PRESENT == 1) || (GSM_TIME_SYNC_PRESENT == 1)
static void sync_time_through_GPS_or_GSM_interleaved(InstaMsg *c)
{
    int rc = FAILURE;
    int maxIterations = 0;

    if(timeSyncedViaExternalResources == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sTime has already been synced via NTP.. not re-syncing."), CLOCK);
        info_log(LOG_GLOBAL_BUFFER);

        return;
    }

    if(gpsGsmTimeSyncFeatureEnabled == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sGPS/GSM-Time-Sync is ENABLED."), CLOCK);
        info_log(LOG_GLOBAL_BUFFER);
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sGPS/GSM-Time-Sync is DISABLED."), CLOCK);
        info_log(LOG_GLOBAL_BUFFER);

        return;
    }


    maxIterations = (sg_atoi(maxSecondsWaitForGpsGsmTimeSync) / MAX_TIME_ALLOWED_FOR_ONE_GPS_ITERATION);
    {
        int i = 0;
        for(i = 0; i < maxIterations; i++)
        {
            unsigned long currentTick = getCurrentTick();
            int remainingSeconds = MAX_TIME_ALLOWED_FOR_ONE_GPS_ITERATION;

#if GPS_TIME_SYNC_PRESENT == 1
            /*
             * As of now, it seems that the whole universe uses NMEA as the de-facto standard for GPS.
             * So, till we see something new, we assume that NMEA-sentences are our source of inspiration.
             */
            get_gps_sentence(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), GPS_TIME_SYNC_SENTENCE_TYPE);

            remainingSeconds = MAX_TIME_ALLOWED_FOR_ONE_GPS_ITERATION - (getCurrentTick() - currentTick);
            if(strlen((char*)GLOBAL_BUFFER) == 0)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s[GPS-Iteration-%u/%u] %s-sentence could not be fetched from NMEA-blob."),
                           CLOCK_ERROR, i + 1, maxIterations, GPS_TIME_SYNC_SENTENCE_TYPE);
                error_log(LOG_GLOBAL_BUFFER);

                goto try_syncing_with_gsm;
            }

            rc = fill_in_time_coordinates_from_sentence((char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), &dateParams, GPS_TIME_SYNC_SENTENCE_TYPE);
            if(rc != SUCCESS)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s[GPS-Iteration-%u/%u] Time-coordinates could not be fetched from GPS."),
                           CLOCK_ERROR, i + 1, maxIterations);
                error_log(LOG_GLOBAL_BUFFER);

                goto try_syncing_with_gsm;
            }

            rc = sync_system_clock(&dateParams, 0);
            if(rc != SUCCESS)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER,
                           PROSTR("%s[GPS-Iteration-%u/%u] Time-coordinates fetched from GPS, but failed to sync time with system-clock."),
                           CLOCK_ERROR, i + 1, maxIterations);
                error_log(LOG_GLOBAL_BUFFER);

                goto try_syncing_with_gsm;
            }
            else
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s[GPS-Iteration-%u/%u] Time-Synced Successfully through GPS."),
                           CLOCK, i + 1, maxIterations);
                info_log(LOG_GLOBAL_BUFFER);

                timeSyncedViaExternalResources = 1;
                break;
            }
#endif

try_syncing_with_gsm:
#if GSM_TIME_SYNC_PRESENT ==1
            timestampFromGSM = get_GSM_timestamp();
            if(timestampFromGSM == 0)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s[GSM-Iteration-%u/%u] Timestamp not available from GSM."),
                           CLOCK_ERROR, i + 1, maxIterations);
                error_log(LOG_GLOBAL_BUFFER);

                goto failure_while_syncing_through_gsm;
            }

            extract_date_params(timestampFromGSM, &dateParams, "GSM");

            rc = sync_system_clock(&dateParams, 0);
            if(rc != SUCCESS)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER,
                           PROSTR("%s[GSM-Iteration-%u/%u] Timestamp fetched from GSM, but failed to sync time with system-clock."),
                           CLOCK_ERROR, i + 1, maxIterations);
                error_log(LOG_GLOBAL_BUFFER);

                goto failure_while_syncing_through_gsm;
            }
            else
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s[GSM-Iteration-%u/%u] Time-Synced Successfully through GSM."),
                           CLOCK, i + 1, maxIterations);
                info_log(LOG_GLOBAL_BUFFER);

                timeSyncedViaExternalResources = 1;
                break;
            }


failure_while_syncing_through_gsm:
#endif
            startAndCountdownTimer(remainingSeconds, 0);
        }
    }

    if(timeSyncedViaExternalResources == 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFailed to sync-time through NTP/GPS/GSM, no point proceeding further ..."), CLOCK_ERROR);
        error_log(LOG_GLOBAL_BUFFER);

        exitApp(0);
    }
}
#endif


#if NTP_TIME_SYNC_PRESENT == 1
static void sync_time_through_NTP(InstaMsg *c)
{
    int rc = FAILURE;

    unsigned long seconds1970 = 0x83aa7e80;   /* number of seconds from 1900 to 1970 */
    unsigned long seconds1900;                /* number of seconds from 1900         */

    if(ntpTimeSyncFeatureEnabled == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sNTP-Time-Sync is ENABLED."), CLOCK);
        info_log(LOG_GLOBAL_BUFFER);
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sNTP-Time-Sync is DISABLED."), CLOCK);
        info_log(LOG_GLOBAL_BUFFER);

        return;
    }

    if(timeSyncedViaExternalResources == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sTime has already been synced earlier.. not re-syncing."), CLOCK);
        info_log(LOG_GLOBAL_BUFFER);

        return;
    }

    (c->timeSyncerSocket).socketCorrupted = 1;

	init_socket(&(c->timeSyncerSocket), ntpServer, NTP_PORT, SOCKET_UDP, 0);
    if((c->timeSyncerSocket).socketCorrupted == 1)
    {
        goto failure_in_time_syncing;
    }

    /*
     * Send the NTP-packet.
     */
    memset(ntpPacket, 0, sizeof(ntpPacket));
    ntpPacket[0] = 0x0b;

    watchdog_reset_and_enable(60, "sending-ntp-packet-to-ntp-server", 1);
    rc = (c->timeSyncerSocket).write(&(c->timeSyncerSocket), ntpPacket, sizeof(ntpPacket));
    if(rc != SUCCESS)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFailed to send NTP-Packet."), CLOCK_ERROR);
        error_log(LOG_GLOBAL_BUFFER);

        goto failure_in_time_syncing;
    }

    watchdog_disable(NULL, NULL);


    /*
     * Wait for response.
     */
    watchdog_reset_and_enable(60 * MAX_TRIES_ALLOWED_WHILE_READING_FROM_SOCKET_MEDIUM * SOCKET_READ_TIMEOUT_SECS,
                              PROSTR("reading-ntp-packet-from-ntp-server"), 1);

    memset(messageBuffer, 0, sizeof(messageBuffer));
    rc = (c->timeSyncerSocket).read(&(c->timeSyncerSocket), (unsigned char*) messageBuffer, 48, 1);
    if(rc != SUCCESS)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFailed to read NTP-Packet."), CLOCK_ERROR);
        error_log(LOG_GLOBAL_BUFFER);

        goto failure_in_time_syncing;
    }

    watchdog_disable(NULL, NULL);

    {
        int i = 0;
        seconds1900 = 0;

        for(i = 0; i < (unsigned char)(messageBuffer[40]); i++)
        {
            seconds1900 = seconds1900 + 16777216;
        }

        for(i = 0; i < (unsigned char)(messageBuffer[41]); i++)
        {
            seconds1900 = seconds1900 + 65536;
        }

        for(i = 0; i < (unsigned char)(messageBuffer[42]); i++)
        {
            seconds1900 = seconds1900 + 256;
        }

        seconds1900 = seconds1900 + (unsigned char)(messageBuffer[43]);
    }

    extract_date_params(seconds1900 - seconds1970, &dateParams, "NTP");

    rc = sync_system_clock(&dateParams, seconds1900 - seconds1970);
    if(rc != SUCCESS)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFailed in last step to sync time with system-clock."), CLOCK_ERROR);
        error_log(LOG_GLOBAL_BUFFER);

        goto failure_in_time_syncing;
    }

    /*
     * We do not close the socket, as some devices reboot on restarting the socket.
     * Anyway, we would be good, as long as devices support at least 2 open sockets at a time.
     */

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sTime-Synced Successfully through NTP."), CLOCK);
    info_log(LOG_GLOBAL_BUFFER);

    timeSyncedViaExternalResources = 1;
    return;


failure_in_time_syncing:
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFailed to sync-time through NTP."), CLOCK_ERROR);
        error_log(LOG_GLOBAL_BUFFER);

#if (GSM_TIME_SYNC_PRESENT == 1) || (GPS_TIME_SYNC_PRESENT == 1)
        if(gpsGsmTimeSyncFeatureEnabled == 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER,
                       PROSTR("%sSince GPS/GSM-Time-Syncing is absent/disabled, so no point proceeding further ..."), CLOCK_ERROR);
            error_log(LOG_GLOBAL_BUFFER);

            exitApp(0);
        }
#endif
}
#endif


static void check_if_conditional_features_are_enabled()
{
#if (NTP_TIME_SYNC_PRESENT == 1) || (GPS_TIME_SYNC_PRESENT == 1) || (GSM_TIME_SYNC_PRESENT == 1)
    int rc = FAILURE;
#endif

#if NTP_TIME_SYNC_PRESENT == 1
    RESET_GLOBAL_BUFFER;

    rc = get_config_value_from_persistent_storage(NTP_SERVER, (char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));
    if(rc == SUCCESS)
    {
        memset(ntpServer, 0, sizeof(ntpServer));
        getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, ntpServer);

        if(strlen(ntpServer) > 0)
        {
            ntpTimeSyncFeatureEnabled = 1;
        }
    }
#endif

#if (GPS_TIME_SYNC_PRESENT == 1) || (GSM_TIME_SYNC_PRESENT == 1)
    RESET_GLOBAL_BUFFER;

    rc = get_config_value_from_persistent_storage(MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC, (char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));
    if(rc == SUCCESS)
    {
        memset(maxSecondsWaitForGpsGsmTimeSync, 0, sizeof(maxSecondsWaitForGpsGsmTimeSync));
        getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, maxSecondsWaitForGpsGsmTimeSync);

        if(sg_atoi(maxSecondsWaitForGpsGsmTimeSync) > 0)
        {
            gpsGsmTimeSyncFeatureEnabled = 1;
        }
    }
#endif

#if SEND_GPS_LOCATION == 1
    RESET_GLOBAL_BUFFER;

    rc = get_config_value_from_persistent_storage(SEND_GPS_LOCATION_INTERVAL, (char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));
    if(rc == SUCCESS)
    {
        char small[6] = {0};

        memset(small, 0, sizeof(small));
        getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, small);

        sendGpsLocationInterval = sg_atoi(small);
    }
#endif

}


static void check_if_all_required_compile_time_defines_are_present()
{
#ifndef NTP_TIME_SYNC_PRESENT
#error "NTP_TIME_SYNC_PRESENT compile-time-parameter undefined"
#endif

#ifndef GPS_TIME_SYNC_PRESENT
#error "GPS_TIME_SYNC_PRESENT compile-time-parameter undefined"
#elif   GPS_TIME_SYNC_PRESENT == 1
#ifndef GPS_TIME_SYNC_SENTENCE_TYPE
#error "GPS_TIME_SYNC_SENTENCE_TYPE compile-time-parameter undefined"
#endif
#endif

#ifndef GSM_TIME_SYNC_PRESENT
#error "GSM_TIME_SYNC_PRESENT compile-time-parameter undefined"
#endif
#ifndef SEND_GPS_LOCATION
#error "SEND_GPS_LOCATION compile-time-parameter undefined"
#elif SEND_GPS_LOCATION == 1
#ifndef GPS_LOCATION_SENTENCE_TYPE
#error "GPS_LOCATION_SENTENCE_TYPE compile-time-parameter undefined"
#endif
#endif

#ifndef DATA_LOGGING_ENABLED
#error "DATA_LOGGING_ENABLED compile-time-parameter undefined"
#endif

#ifndef SSL_ENABLED
#error "SSL_ENABLED compile-time-parameter undefined"
#endif

#ifndef AT_INTERFACE_ENABLED
#error "AT_INTERFACE_ENABLED compile-time-parameter undefined"
#endif

#ifndef SOCKET_SSL_ENABLED
#error "SOCKET_SSL_ENABLED compile-time-parameter undefined"
#endif

#ifndef ENSURE_EXPLICIT_TIME_SYNC
#error "ENSURE_EXPLICIT_TIME_SYNC compile-time-parameter undefined"
#endif
}


void initInstaMsg(InstaMsg* c,
                  int (*connectHandler)(),
                  int (*disconnectHandler)(),
                  int (*oneToOneMessageHandler)())
{
    int i;

    runBusinessLogicImmediately = 0;

    /*
     * At the very start, start this to true.
     */
    actuallyEnsureGuaranteeWhereRequired = 1;

    c->FRESH_SERVER_LOGS_TIME = -1;

#if FILE_SYSTEM_ENABLED == 1
    init_file_system(&(c->singletonUtilityFs), "");
    deleteFile(&(c->singletonUtilityFs), SYSTEM_WIDE_TEMP_FILE);
#endif

    check_for_upgrade();
    check_if_all_required_compile_time_defines_are_present();


    check_if_conditional_features_are_enabled();

#if NTP_TIME_SYNC_PRESENT == 1
    sync_time_through_NTP(c);
#endif
#if GPS_TIME_SYNC_PRESENT == 1
    sync_time_through_GPS_or_GSM_interleaved(c);
#endif

    (c->ipstack).socketCorrupted = 1;
	init_socket(&(c->ipstack), INSTAMSG_HOST, INSTAMSG_PORT, SOCKET_TCP, 1);
    if((c->ipstack).socketCorrupted ==1)
    {
        handleConnOrProvAckGeneric(c, 0, SIMULATED);
    }


    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        c->messageHandlers[i].msgId = 0;
        c->messageHandlers[i].topicFilter = 0;

        c->resultHandlers[i].msgId = 0;
        c->resultHandlers[i].timeout = 0;

        c->oneToOneHandlers[i].msgId = 0;
        c->oneToOneHandlers[i].timeout = 0;
    }

    c->defaultMessageHandler = NULL;
    c->next_packetid = MAX_PACKET_ID;
    c->onConnectCallback = connectHandler;
    c->onDisconnectCallback = disconnectHandler;
    c->oneToOneMessageHandler = oneToOneMessageHandler;

    c->serverLoggingEnabled = 0;

	c->connectOptions.willFlag = 0;
	c->connectOptions.MQTTVersion = 3;
	c->connectOptions.cleansession = 1;


    memset(c->clientIdComplete, 0, sizeof(c->clientIdComplete));
    strcpy(c->clientIdComplete, "");

    c->connected = 0;

    pubAckRecvAttempts = 0;
    waitingForPuback = NOT_WAITING_FOR_PUBACK;
    rebootPending = 0;

#if MEDIA_STREAMING_ENABLED == 1
    mediaReplyReceived = 0;
#endif

    MQTTConnect(c);
}


static void sendClientData(void (*func)(char *messageBuffer, int maxBufferLength),
                          const char *topicName)
{
    /*
     * This method sends the data upon client's connect.
     *
     * If the message(s) are not sent from this method, that means that the connection is not (fully) completed.
     * Thus, the InstaMsg-Driver code will try again for the connection, and then these messages will be sent (again).
     *
     * Bottom-line : We do not need to re-attempt the message(s) sent by this method.
     */

    memset(messageBuffer, 0, sizeof(messageBuffer));
    func(messageBuffer, sizeof(messageBuffer));

    if(strlen(messageBuffer) > 0)
    {
        publish(topicName,
                messageBuffer,
                QOS0,
                0,
                NULL,
                MQTT_RESULT_HANDLER_TIMEOUT,
                1);
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Not publishing empty-message to topic [%s]"), topicName);
        info_log(LOG_GLOBAL_BUFFER);
    }
}


static int send_previously_unsent_data()
{
    int rc = FAILURE;

    /*
     * Also, try sending the records stored in the persistent-storage (if any).
     */
    if(1)
    {
        memset(messageBuffer, 0, sizeof(messageBuffer));
        rc = get_next_record_from_persistent_storage(messageBuffer, sizeof(messageBuffer));

        if(rc == SUCCESS)
        {
            /*
             * We got the record.
             */

            char *topic = NULL;
            char *payload = NULL;

            topic = (char*) sg_malloc(MAX_BUFFER_SIZE);
            payload = (char*) sg_malloc(sizeof(messageBuffer));

            if((topic == NULL) || (payload == NULL))
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not allocate memory in data-logging.. not proceeding"), DATA_LOGGING_ERROR);
                error_log(LOG_GLOBAL_BUFFER);

                goto exit;
            }

            memset(topic, 0, MAX_BUFFER_SIZE);
            memset(payload, 0, sizeof(messageBuffer));

            getJsonKeyValueIfPresent(messageBuffer, DATA_LOG_TOPIC, topic);
            getJsonKeyValueIfPresent(messageBuffer, DATA_LOG_PAYLOAD, payload);

            startAndCountdownTimer(1, 0);

            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Sending data that could not be sent earlier [%s] over [%s]"), payload, topic);
            info_log(LOG_GLOBAL_BUFFER);

            if(publishMessageWithDeliveryGuarantee(topic, payload) != SUCCESS)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER,
                           PROSTR("Since the data could not be sent to InstaMsg-Server, so not retrying sending data from persistent-storage"));
                error_log(LOG_GLOBAL_BUFFER);
            }

exit:
            if(payload)
                sg_free(payload);

            if(topic)
                sg_free(topic);
        }
        else
        {
            /*
             * We did not get any record.
             */
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("\n\nNo more pending-data to be sent from persistent-storage\n\n"));
            info_log(LOG_GLOBAL_BUFFER);
        }
    }

    return rc;
}


static void handleConnOrProvAckGeneric(InstaMsg *c, int connack_rc, const char *mode)
{
    if((connack_rc == 0x00) || (connack_rc == 0x06))  /* Connection Accepted */
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s successfully to InstaMsg-Server."), mode);
        info_log(LOG_GLOBAL_BUFFER);

        c->connected = 1;

        if(strcmp(mode, SIMULATED) == 0)
        {
            msgSource = GENERAL;
        }
        else
        {
            msgSource = PERSISTENT_STORAGE;
        }

        sendClientData(get_client_session_data, TOPIC_SESSION_DATA);
        sendClientData(get_client_metadata, TOPIC_METADATA);
        sendClientData(get_network_data, TOPIC_NETWORK_DATA);

        registerEditableConfig(&pingRequestInterval,
                               PROSTR("PING_REQ_INTERVAL"),
                               CONFIG_INT,
                               PROSTR("180"),
                               PROSTR("Keep-Alive Interval between Device and InstaMsg-Server"));

        registerEditableConfig(&compulsorySocketReadAfterMQTTPublishInterval,
                               PROSTR("COMPULSORY_SOCKET_READ_AFTER_WRITE_COUNT"),
                               CONFIG_INT,
                               DEFAULT_COMPULSORY_SOCKET_READ_AFTER_WRITE_TIMEOUT,
                               PROSTR("This variable controls after how many MQTT-Publishes a compulsory socket-read is done. This prevents any socket-pverrun errors (particularly in hardcore embedded-devices"));

        {
            char interval[6];
            int *ptr = (int *)(&editableBusinessLogicInterval);
            memset(interval, 0, sizeof(interval));

            sg_sprintf(interval, "%d", editableBusinessLogicInterval);
            registerEditableConfig(ptr,
                                   PROSTR("BUSINESS_LOGIC_INTERVAL"),
                                   CONFIG_INT,
                                   interval,
                                   PROSTR("Business-Logic Interval (in seconds)"));
        }

        registerEditableConfig(&actuallyEnsureGuaranteeWhereRequired,
                               PROSTR("ENABLE_MESSAGE_ACK"),
                               CONFIG_INT,
                               PROSTR("1"),
                               PROSTR(""));

        registerEditableConfig(&debugLoggingEnabled,
                               DEBUG_LOGGING_ENABLED,
                               CONFIG_INT,
                               PROSTR("1"),
                               PROSTR(""));

#if FILE_SYSTEM_ENABLED == 1
        registerEditableConfig(&autoUpgradeEnabled,
                               AUTO_UPGRADE_ENABLED,
                               CONFIG_INT,
                               PROSTR("1"),
                               PROSTR(""));
#endif

        /*
         * Although we took the decision to enable/disable GPS and NTP earlier in the flow, yet we do the following,
         * to make the GPS-Time-Sync and NTP-Server configurable-parameters from the server.
         */
#if (GPS_TIME_SYNC_PRESENT == 1) || (GSM_TIME_SYNC_PRESENT == 1)
        memset(maxSecondsWaitForGpsGsmTimeSync, 0, sizeof(maxSecondsWaitForGpsGsmTimeSync));
        registerEditableConfig(maxSecondsWaitForGpsGsmTimeSync,
                               MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC,
                               CONFIG_STRING,
                               PROSTR(DEFAULT_MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC),
                               PROSTR(""));
#endif

#if NTP_TIME_SYNC_PRESENT == 1
        memset(ntpServer, 0, sizeof(ntpServer));
        registerEditableConfig(ntpServer,
                               NTP_SERVER,
                               CONFIG_STRING,
                               PROSTR(DEFAULT_NTP_SERVER),
                               PROSTR(""));
#endif

#if SEND_GPS_LOCATION == 1
        registerEditableConfig(&sendGpsLocationInterval,
                               SEND_GPS_LOCATION_INTERVAL,
                               CONFIG_INT,
                               PROSTR(DEFAULT_SEND_GPS_LOCATION_INTERVAL),
                               PROSTR(""));
#endif

#if (SSL_ENABLED == 1) || (SOCKET_SSL_ENABLED == 1)
        registerEditableConfig(&sslEnabledAtAppLayer,
                               SSL_ACTUALLY_ENABLED,
                               CONFIG_INT,
                               DEFAULT_SSL_ENABLED,
                               PROSTR(""));
#endif

#if MEDIA_STREAMING_ENABLED == 1
        registerEditableConfig(&mediaStreamingEnabledRuntime,
                               PROSTR("MEDIA_STREAMING_ENABLED"),
                               CONFIG_INT,
                               "0",
                               PROSTR("0 - Disabled; 1 - Enabled"));
        registerEditableConfig(&mediaReplyMessageWaitInterval,
                               PROSTR("MEDIA_REPLY_MESSAGE_WAIT_INTERVAL"),
                               CONFIG_INT,
                               "120",
                               PROSTR(""));

        if(mediaStreamingEnabledRuntime == 1)
        {
            static unsigned char streamingInitDone = 0;
            if(streamingInitDone == 0)
            {
                initiateStreaming();
                streamingInitDone = 1;
            }
        }
#endif

#if HTTP_PROXY_ENABLED == 1
        memset(c->proxyEndUnitServerAndPort, 0, sizeof(c->proxyEndUnitServerAndPort));
        registerEditableConfig(c->proxyEndUnitServerAndPort,
                               PROXY_END_SERVER_PORT,
                               CONFIG_STRING,
                               PROSTR(""),
                               PROSTR(""));

        if(strlen(c->proxyEndUnitServerAndPort) > 0)
        {
            int attempts = 0;

            while(1)
            {
                if(proxyParamsReceived == 0)
                {
                    if(attempts >= 20)
                    {
                        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sProxy-Params not received, exiting .."), PROXY_ERROR);
                        error_log(LOG_GLOBAL_BUFFER);

                        exitApp(0);
                    }

                    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sProxy-Params still not received, waiting ..."), PROXY);
                    info_log(LOG_GLOBAL_BUFFER);

                    readAndProcessIncomingMQTTPacketsIfAny(c);
                    attempts++;
                }
                else
                {
                    break;
                }
            }


            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sProxy-Params received successfully !!!"), PROXY);
            info_log(LOG_GLOBAL_BUFFER);

            setupProxy(c);
        }
#endif

        if(notifyServerOfSecretReceived == 1)
        {
            sendPacketIrrespective = 1;
            publish(NOTIFICATION_TOPIC,
                    "SECRET RECEIVED",
                    QOS0,
                    0,
                    NULL,
                    MQTT_RESULT_HANDLER_TIMEOUT,
                    1);
        }

        if(c->onConnectCallback != NULL)
        {
            c->onConnectCallback();
            c->onConnectCallback = NULL;
        }
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Client-%s failed with code [%d]"), mode, connack_rc);
        error_log(LOG_GLOBAL_BUFFER);

        exitApp(0);
    }
}


static void process_received_client_id(InstaMsg *c, void *buf)
{
    memset(c->clientIdComplete, 0, sizeof(c->clientIdComplete));
    memcpy(c->clientIdComplete, buf, 36);

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Received client-id from server via PROVACK [%s]"), c->clientIdComplete);
    info_log(LOG_GLOBAL_BUFFER);
}


static void process_received_secret(InstaMsg *c)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Received client-secret from server via PROVACK [%s]"), c->password);
    info_log(LOG_GLOBAL_BUFFER);
}



void readAndProcessIncomingMQTTPacketsIfAny(InstaMsg* c)
{
    int rc = FAILURE;
    do
    {
        int len = 0;
        MQTTFixedHeader fixedHeader;

        rc = readPacket(c, &fixedHeader);
        if(rc != SUCCESS)
        {
            return;
        }

        switch (fixedHeader.packetType)
        {
            case CONNACK:
            {
                unsigned char connack_rc = 255;
                char sessionPresent = 0;
                if (MQTTDeserialize_connack((unsigned char*)&sessionPresent, &connack_rc, c->readbuf, sizeof(c->readbuf)) == 1)
                {
                    handleConnOrProvAckGeneric(c, connack_rc, CONNECTED);
                }

                break;
            }

            case PROVACK:
            {
                MQTTMessage msg;
                unsigned char connack_rc = 255;
                char sessionPresent = 0;
                if (MQTTDeserialize_provack((unsigned char*)&sessionPresent,
                                             &connack_rc,
                                             (unsigned char**)&msg.payload,
                                             (int*)&msg.payloadlen,
                                             c->readbuf,
                                             sizeof(c->readbuf)) == 1)
                {
                    if(connack_rc == 0x00)  /* Provision-Successful-Without-Certificate */
                    {
                        memcpy(c->clientIdComplete, msg.payload, msg.payloadlen);

                        {
                            int remainingPayloadLength = msg.payloadlen - 36;
                            if(remainingPayloadLength > 1)
                            {
                                memset(c->password, 0, sizeof(c->password));
                                memcpy(c->password, (char*)msg.payload + 37, remainingPayloadLength - 1);
                                process_received_secret(c);

                                memset(messageBuffer, 0, sizeof(messageBuffer));
                                generate_config_json(messageBuffer, SECRET, CONFIG_STRING, c->clientIdComplete, "");
                                save_config_value_on_persistent_storage(SECRET, messageBuffer, 1);

                                /*
                                 * Send notification to the server, that the secret-password has been saved.
                                 */
                                notifyServerOfSecretReceived = 1;
                           }
                        }

                        process_received_client_id(c, msg.payload);
                        exitApp(0);
                    }
                    else if(connack_rc == 0x06) /* Provision-Successful-With-Certificate */
                    {
#if 0
                        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Received payload = [%s]"), (char*)(msg.payload));
                        info_log(LOG_GLOBAL_BUFFER);
#endif
                        char *temp = NULL;
                        int sz = 2000;

                        temp = (char*) sg_malloc(2000);
                        if(temp == NULL)
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory for processing provisioning-response, bye ..");
                            error_log(LOG_GLOBAL_BUFFER);

                            exitApp(0);
                        }

                        memset(temp, 0, sz);
                        getJsonKeyValueIfPresent(msg.payload, "client_id", temp);
                        process_received_client_id(c, temp);

                        memset(temp, 0, sz);
                        getJsonKeyValueIfPresent(msg.payload, "auth_token", temp);
                        memset(c->password, 0, sizeof(c->password));
                        strcpy(c->password, temp);
                        process_received_secret(c);

                        memset(temp, 0, sz);
                        strcat(temp, c->clientIdComplete);
                        strcat(temp, "-");
                        strcat(temp, c->password);
                        memset(messageBuffer, 0, sizeof(messageBuffer));
                        generate_config_json(messageBuffer, SECRET, CONFIG_STRING, temp, "");
                        save_config_value_on_persistent_storage(SECRET, messageBuffer, 1);

                        sg_free(temp);

                        processCertificateInfoIfAny(c, (char*)(msg.payload));
                        exitApp(0);
                    }

                    handleConnOrProvAckGeneric(c, connack_rc, PROVISIONED);
                }

                break;
            }

            case PUBACK:
            {
                fireResultHandlerUsingMsgIdAsTheKey(c);
                break;
            }

            case SUBACK:
            {

                /*
                * Remove the message-handlers, if the server was unable to process the subscription-request.
                */
                int count = 0, grantedQoS = -1;
                unsigned short msgId;

                fireResultHandlerUsingMsgIdAsTheKey(c);

                if (MQTTDeserialize_suback(&msgId, 1, &count, &grantedQoS, c->readbuf, sizeof(c->readbuf)) != 1)
                {
                    goto exit;
                }

                if (grantedQoS == 0x80)
                {
                    int i;
                    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
                    {
                        if (c->messageHandlers[i].msgId == msgId)
                        {
                            c->messageHandlers[i].topicFilter = 0;
                            break;
                        }
                    }
                }

                break;
            }

            case PUBLISH:
            {
                MQTTString topicMQTTString;
                MQTTMessage msg;
                char *topicName;
                unsigned char memoryAllocatedSynamicaaly = 0;

                if (MQTTDeserialize_publish(&(msg.fixedHeaderPlusMsgId),
                                            &topicMQTTString,
                                            (unsigned char**)&msg.payload,
                                            (int*)&msg.payloadlen,
                                            c->readbuf,
                                            sizeof(c->readbuf)) != SUCCESS)
                {
                    goto exit;
                }

                /*
                 * At this point, "msg.payload" contains the real-stuff that is passed from the peer ....
                 */
                topicName = topicMQTTString.lenstring.data;

                /*
                 * Sometimes, topic-name and payload are not separated by above algo.
                 * So, do another check
                 */
                if(strstr(topicName, msg.payload) != NULL)
                {
                    topicName = (char*) sg_malloc(MAX_BUFFER_SIZE);
                    if(topicName == NULL)
                    {
                        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Could not allocate memory for topic"));
                        error_log(LOG_GLOBAL_BUFFER);

                        goto publish_exit;
                    }
                    else
                    {
                        memset(topicName, 0, MAX_BUFFER_SIZE);

                        memoryAllocatedSynamicaaly = 1;
                        strncpy(topicName, topicMQTTString.lenstring.data, strlen(topicMQTTString.lenstring.data) - msg.payloadlen);
                    }
                }

                if(topicName != NULL)
                {
                    if(strcmp(topicName, c->filesTopic) == 0)
                    {
                        handleFileTransfer(c, &msg);
                    }
                    else if(strcmp(topicName, c->enableServerLoggingTopic) == 0)
                    {
                        serverLoggingTopicMessageArrived(c, &msg);
                    }
                    else if(strcmp(topicName, c->rebootTopic) == 0)
                    {
                        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Received REBOOT request from server.. rebooting !!!"));
                        info_log(LOG_GLOBAL_BUFFER);

                        exitApp(0);
                    }
                    else if(strcmp(topicName, c->clientIdComplete) == 0)
                    {
                        oneToOneMessageArrived(c, &msg);
                    }
                    else if(strcmp(topicName, c->receiveConfigTopic) == 0)
                    {
                        handleConfigReceived(c, &msg);
                    }
                    else if(strcmp(topicName, c->updateCertTopic) == 0)
                    {
                        handleCertReceived(c, &msg);
                    }
#if MEDIA_STREAMING_ENABLED == 1
                    else if(strcmp(topicName, c->mediaReplyTopic) == 0)
                    {
                        handleMediaReplyMessage(c, &msg);
                    }
                    else if(strcmp(topicName, c->mediaStreamsTopic) == 0)
                    {
                        handleMediaStreamsMessage(c, &msg);
                    }
                    else if(strcmp(topicName, c->mediaStopTopic) == 0)
                    {
                        handleMediaStopMessage(c);
                    }
                    else if(strcmp(topicName, c->mediaPauseTopic) == 0)
                    {
                        handleMediaPauseMessage(c);
                    }
#endif
#if HTTP_PROXY_ENABLED == 1
                    else if(strcmp(topicName, c->proxyTopic) == 0)
                    {
                        handleProxyMessage(c, &msg);
                    }
#endif
                    else
                    {
                        deliverMessageToSelf(c, &topicMQTTString, &msg);
                    }
                }
                else
                {
                    deliverMessageToSelf(c, &topicMQTTString, &msg);
                }

publish_exit:
                if(memoryAllocatedSynamicaaly == 1)
                {
                    if(topicName)
                        sg_free(topicName);
                }

                break;
            }

            case PUBREC:
            {
                int msgId = fireResultHandlerUsingMsgIdAsTheKey(c);

                RESET_GLOBAL_BUFFER;
                if ((len = MQTTSerialize_ack(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), PUBREL, 0, msgId)) <= 0)
                {
                    goto exit;
                }

                attachResultHandler(c, msgId, MQTT_RESULT_HANDLER_TIMEOUT, publishQoS2CycleCompleted);
                sendPacket(c, &(c->ipstack), GLOBAL_BUFFER, len); /* send the PUBREL packet */

                break;
            }

            case PUBCOMP:
            {
                fireResultHandlerUsingMsgIdAsTheKey(c);
                break;
            }

            case PINGRESP:
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("PINGRESP received... relations are intact !!\n"));
                info_log(LOG_GLOBAL_BUFFER);

                pingReqResponsePending = 0;

                break;
            }
        }
    } while(rc == SUCCESS); /* Keep reading packets till the time we are receiving packets fine. */

exit:
        return;
}


void* MQTTConnect(void* arg)
{
    int len = 0;
    int rc;
    char *secret = NULL;
    InstaMsg *c = (InstaMsg *)arg;

    memset(c->clientIdMachine, 0, sizeof(c->clientIdMachine));
    memset(c->username, 0, sizeof(c->username));
    memset(c->password, 0, sizeof(c->password));

    RESET_GLOBAL_BUFFER;

    rc = get_config_value_from_persistent_storage(SECRET, (char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));
    if(rc == SUCCESS)
    {
        /*
         * We will receive CONNACK for this leg.
         */
        secret = (char*) sg_malloc(MAX_BUFFER_SIZE);
        if(secret == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not allocate memory for secret .. not proceeding further", CONFIG);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

        memset(secret, 0, MAX_BUFFER_SIZE);
        getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, secret);

        memset(c->clientIdComplete, 0, sizeof(c->clientIdComplete));
        memcpy(c->clientIdComplete, secret, 36);
        setValuesOfSpecialTopics(c);

        strcpy(c->clientIdMachine, EMPTY_CLIENT_ID);
        memcpy(c->username, secret, 36);
        memcpy(c->password, secret + 37, strlen(secret) - 37);

        notifyServerOfSecretReceived = 1;
    }
    else
    {
        /*
         * We will receive PROVACK for this leg.
         */
        strcpy(c->clientIdMachine, PROVISIONING_CLIENT_ID);
#if GSM_INTERFACE_ENABLED == 1
        strcpy(c->password, (c->ipstack).provPin);
#else
        get_prov_pin_for_non_gsm_devices(c->password, sizeof(c->password));
#endif
        get_device_uuid(c->username, sizeof(c->username));
    }

    c->connectOptions.clientID.cstring = c->clientIdMachine;
    c->connectOptions.username.cstring = c->username;
    c->connectOptions.password.cstring = c->password;

    RESET_GLOBAL_BUFFER;
    if ((len = MQTTSerialize_connect(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), &(c->connectOptions))) <= 0)
    {
        goto exit;
    }

    sendPacketIrrespective = 1;
    sendPacket(c, &(c->ipstack), GLOBAL_BUFFER, len);

exit:
    if(secret)
    {
        sg_free(secret);
    }

    return NULL;
}


static void saveFailedPublishedMessage()
{
    rebootPending = 1;

    if(1)
    {
        if(1)
        {
            int messageJsonSize = sizeof(messageBuffer) + 200;

            char *messageSavingJson = (char*) sg_malloc(messageJsonSize);
            if(messageSavingJson == NULL)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not allocate memory for data-logging .. returning"), DATA_LOGGING_ERROR);
                error_log(LOG_GLOBAL_BUFFER);

                return;
            }

            memset(messageSavingJson, 0, messageJsonSize);
            sg_sprintf(messageSavingJson, "{'%s' : '%s', '%s' : '%s'}",
                       DATA_LOG_TOPIC, lastPubTopic,
                       DATA_LOG_PAYLOAD, lastPubPayload);

#if DATA_LOGGING_ENABLED == 1
            save_record_to_persistent_storage(messageSavingJson);
#endif
            if(messageSavingJson)
                sg_free(messageSavingJson);

            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sEither message-sending failed over wire, "
                                                 "or PUBACK was not received for message [%s] within time"),
                       DATA_LOGGING_ERROR, lastPubPayload);
            error_log(LOG_GLOBAL_BUFFER);

            waitingForPuback = PUBACK_TIMEOUT;
            freeLastPubMessageResources();
        }
    }
}


int subscribe(const char* topicName,
              const int qos,
              void (*messageHandler)(MessageData *),
              void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
              unsigned int resultHandlerTimeout,
              const char logging)
{
    int rc = FAILURE;
    int len = 0;
    int id;
    InstaMsg *c = &instaMsg;

    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;

    if(logging == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Subscribing to topic [%s]"), topicName);
        info_log(LOG_GLOBAL_BUFFER);
    }

    RESET_GLOBAL_BUFFER;

    id = getNextPacketId(c);
    len = MQTTSerialize_subscribe(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), 0, id, 1, &topic, (int*)&qos);
    if (len <= 0)
        goto exit;

    attachResultHandler(c, id, resultHandlerTimeout, resultHandler);

    /*
     * We follow optimistic approach, and assume that the subscription will be successful, and accordingly assign the
     * message-handlers.
     *
     * If the subscription is unsuccessful, we would then remove/unsubscribe the topic.
     */
    {
        int i;

        for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        {
            if (c->messageHandlers[i].topicFilter == 0)
            {
                c->messageHandlers[i].msgId = id;
                c->messageHandlers[i].topicFilter = topicName;
                c->messageHandlers[i].fp = messageHandler;

                break;
            }
         }
    }

    startAndCountdownTimer(1, 0);
    if ((rc = sendPacket(c, &(c->ipstack), GLOBAL_BUFFER, len)) != SUCCESS) /* send the subscribe packet */
        goto exit;             /* there was a problem */

exit:

    if(logging == 1)
    {
        if(rc == SUCCESS)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Subscribed successfully.\n"));
            info_log(LOG_GLOBAL_BUFFER);
        }
        else
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Subscribing failed, error-code = [%d]\n"), rc);
            info_log(LOG_GLOBAL_BUFFER);
        }
    }

    return rc;
}


int MQTTUnsubscribe(const char* topicFilter)
{
    int rc = FAILURE;
    int len = 0;
    InstaMsg *c = &instaMsg;

    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;

    RESET_GLOBAL_BUFFER;

    if ((len = MQTTSerialize_unsubscribe(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), 0, getNextPacketId(c), 1, &topic)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, &(c->ipstack), GLOBAL_BUFFER, len)) != SUCCESS) /* send the subscribe packet */
        goto exit; /* there was a problem */

exit:
    return rc;
}


static void waitForPubAck()
{
    if(1)
    {
        if(1)
        {
            if(1)
            {
                while(1)
                {
                    readAndProcessIncomingMQTTPacketsIfAny(&instaMsg);
                    if(waitingForPuback == WAITING_FOR_PUBACK)
                    {
                        pubAckRecvAttempts = pubAckRecvAttempts + 1;
                        if(pubAckRecvAttempts >= MAX_CYCLES_TO_WAIT_FOR_PUBACK)
                        {
                            pubAckRecvAttempts = 0;
                            saveFailedPublishedMessage();

                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
}


static void waitForPingResp()
{
    if(1)
    {
        if(1)
        {
            if(1)
            {
                while(1)
                {
                    readAndProcessIncomingMQTTPacketsIfAny(&instaMsg);
                    if(pingReqResponsePending == 1)
                    {
                        pingRespAttempts = pingRespAttempts + 1;
                        if(pingRespAttempts >= MAX_CYCLES_TO_WAIT_FOR_PINGRESP)
                        {
                            pingRespAttempts = 0;

                            sg_sprintf(LOG_GLOBAL_BUFFER, "Previous response for PINGREQ not received, device will be rebooted ..");
                            error_log(LOG_GLOBAL_BUFFER);

                            rebootPending = 1;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
}


int publish(const char* topicName,
            const char* payload,
            const int qos,
            const char dup,
            void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
            unsigned int resultHandlerTimeout,
            const char logging)
{
    static unsigned int publishCount = 0;
    int rc = FAILURE;
    int len = 0;
    int id = -1;
    InstaMsg *c = &instaMsg;

    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;

    startAndCountdownTimer(1, 0);
    RESET_GLOBAL_BUFFER;

    if (qos == QOS1 || qos == QOS2)
    {
        id = getNextPacketId(c);

        pubAckMsgId = id;
        waitingForPuback = WAITING_FOR_PUBACK;

        lastPubTopic = (char*) sg_malloc(100);
        lastPubPayload = (char*) sg_malloc(sizeof(messageBuffer));

        if((lastPubTopic == NULL) || (lastPubPayload == NULL))
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory to track publish-messages");
            error_log(LOG_GLOBAL_BUFFER);

            return FAILURE;
        }

        memset(lastPubTopic, 0, 100);
        memset(lastPubPayload, 0, sizeof(messageBuffer));

        memcpy(lastPubTopic, topicName, strlen(topicName));
        memcpy(lastPubPayload, payload, strlen(payload));



        /*
         * We will get PUBACK from server only for QOS1 and QOS2.
         * So, it makes sense to lodge the result-handler only for these cases.
         */
        attachResultHandler(c, id, resultHandlerTimeout, resultHandler);
    }

    if(logging == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("\nPublishing message [%s] to topic [%s]"), payload, topicName);
        info_log(LOG_GLOBAL_BUFFER);
    }

    len = MQTTSerialize_publish(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), 0, qos, 0, id, topic, (unsigned char*)payload, strlen((char*)payload));
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, &(c->ipstack), GLOBAL_BUFFER, len)) != SUCCESS) /* send the subscribe packet */
    {
        goto exit; /* there was a problem */
    }
    else
    {
        publishCount++;
    }

    if (qos == QOS1)
    {
    }
    else if (qos == QOS2)
    {
    }

exit:

    if(rc == SUCCESS)
    {
        if(logging == 1)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Published successfully over socket."));
            info_log(LOG_GLOBAL_BUFFER);
        }

        if(compulsorySocketReadAfterMQTTPublishInterval != 0)
        {
            if((publishCount % compulsorySocketReadAfterMQTTPublishInterval) == 0)
            {
                if(logging == 1)
                {
                    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Doing out-of-order socket-read, as [%u] MQTT-Publishes have been done"),
                               compulsorySocketReadAfterMQTTPublishInterval);
                    info_log(LOG_GLOBAL_BUFFER);
                }

                readAndProcessIncomingMQTTPacketsIfAny(c);
            }
        }

        if((qos == QOS1) || (qos == QOS2))
        {
            waitForPubAck();
        }
    }
    else
    {
        if(logging == 1)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Publishing failed over socket.\n"));
            error_log(LOG_GLOBAL_BUFFER);
        }

        if((qos == QOS1) || (qos == QOS2))
        {
            saveFailedPublishedMessage();
        }
   }


    return rc;
}


int publishMessageWithDeliveryGuarantee(char *topic, char *payload)
{
    if(actuallyEnsureGuaranteeWhereRequired == 0)
    {
        return publish(topic,
                       payload,
                       QOS0,
                       0,
                       NULL,
                       MQTT_RESULT_HANDLER_TIMEOUT,
                       1);
    }
    else
    {
        return publish(topic,
                       payload,
                       QOS1,
                       0,
                       publishAckReceived,
                       MQTT_RESULT_HANDLER_TIMEOUT,
                       1);
    }
}


int sendOneToOne(const char* peer,
                 const char* payload,
                 int (*oneToOneHandler)(OneToOneResult *),
                 unsigned int timeout)
{
    InstaMsg *c = &instaMsg;
    int id = getNextPacketId(c);

    memset(messageBuffer, 0, sizeof(messageBuffer));
    sg_sprintf(messageBuffer, PROSTR("{\"message_id\": \"%u\", \"reply_to\": \"%s\", \"body\": \"%s\"}"), id, c->clientIdComplete, payload);

    return doMqttSendPublish(id, oneToOneHandler, timeout, peer, messageBuffer);
}


int MQTTDisconnect(InstaMsg* c)
{
    int rc = FAILURE;
    int len;

    RESET_GLOBAL_BUFFER;

    len = MQTTSerialize_disconnect(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));

    if (len > 0)
        rc = sendPacket(c, &(c->ipstack), GLOBAL_BUFFER, len);            /* send the disconnect packet */

    if(c->onDisconnectCallback != NULL)
    {
        c->onDisconnectCallback();
    }

    return rc;
}


#if SEND_GPS_LOCATION == 1
void sendGpsLocationToServer()
{
    get_gps_sentence(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), GPS_LOCATION_SENTENCE_TYPE);
    if(strlen((char*)GLOBAL_BUFFER) == 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s%s-sentence could not be fetched from NMEA-blob."), GPS_ERROR, GPS_LOCATION_SENTENCE_TYPE);
        error_log(LOG_GLOBAL_BUFFER);

        /*
         * We, however, still send the empty string.
         */
    }

    send_gps_location((char*) GLOBAL_BUFFER);
}
#endif


volatile unsigned long nextBusinessLogicTick;

void start(int (*onConnectOneTimeOperations)(),
           int (*onDisconnect)(),
           int (*oneToOneMessageReceivedHandler)(OneToOneResult *),
           void (*coreLoopyBusinessLogicInitiatedBySelf)(),
           int businessLogicInterval)
{
    InstaMsg *c = &instaMsg;
    int rc = FAILURE;

    volatile unsigned long latestTick = getCurrentTick();
    unsigned long nextSocketTick = latestTick + NETWORK_INFO_INTERVAL;
    unsigned long nextPingReqTick = latestTick + pingRequestInterval;
    unsigned long nextStatsDisplayTick = 0;
#if SEND_GPS_LOCATION == 1
    unsigned long nextSendGpsLocationTick = latestTick + sendGpsLocationInterval;
#endif

    debugLoggingEnabled = 1;
    RESET_GLOBAL_BUFFER;

    rc = get_config_value_from_persistent_storage(DEBUG_LOGGING_ENABLED, (char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));
    if(rc == SUCCESS)
    {
        char small[3] = {0};
        getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, small);

        if(strlen(small) > 0)
        {
            if(sg_atoi(small) == 0)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, "Disabling serial-logging .......");
                info_log(LOG_GLOBAL_BUFFER);

                debugLoggingEnabled = 0;
            }
        }
    }

    statsDisplayInterval = 60;
    nextStatsDisplayTick = latestTick + statsDisplayInterval;

    editableBusinessLogicInterval = businessLogicInterval;
    nextBusinessLogicTick = latestTick + editableBusinessLogicInterval;

    sendPacketIrrespective = 0;
    pingRequestInterval = 0;
    notifyServerOfSecretReceived = 0;

    compulsorySocketReadAfterMQTTPublishInterval = 0;
#if MEDIA_STREAMING_ENABLED == 1
    mediaStreamingErrorOccurred = 0;
#endif

    while(1)
    {
        initInstaMsg(c, onConnectOneTimeOperations, onDisconnect, oneToOneMessageReceivedHandler);

        RESET_GLOBAL_BUFFER;
        get_device_uuid((char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Device-UUID :: [%s]"), (char*)GLOBAL_BUFFER);
        info_log(LOG_GLOBAL_BUFFER);

        RESET_GLOBAL_BUFFER;
        get_device_ip_address((char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("IP-Address :: [%s]"), (char*)GLOBAL_BUFFER);
        info_log(LOG_GLOBAL_BUFFER);

        while(1)
        {
            startAndCountdownTimer(1, 0);

            if((c->ipstack).socketCorrupted == 1)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Socket not available at physical layer .. so nothing can be read from socket."));
                error_log(LOG_GLOBAL_BUFFER);
            }
            else
            {
                if(1)
                {
#if 1
                    while(1)
                    {
                        static unsigned char maxConnectionWaitAttempts = 0;
                        static unsigned char onceTried = 0;

                        if(c->connected == 1)
                        {
                            break;
                        }

                        if(onceTried == 1)
                        {
                            break;
                        }

                        if(c->connected != 1)
                        {
                            maxConnectionWaitAttempts++;
                            readAndProcessIncomingMQTTPacketsIfAny(c);
                        }

                        if(maxConnectionWaitAttempts >= 10)
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER, "No PROVACK/CONNACK received even after %d attempts", maxConnectionWaitAttempts);
                            info_log(LOG_GLOBAL_BUFFER);

                            maxConnectionWaitAttempts = 0;
                            onceTried = 1;

                            break;
                        }
                    }
#endif

                    readAndProcessIncomingMQTTPacketsIfAny(c);

                    if((msgSource == PERSISTENT_STORAGE) && (waitingForPuback != WAITING_FOR_PUBACK) && (rebootPending == 0))
                    {
                        int retrievedFromPersistentStorage = send_previously_unsent_data();
                        if(retrievedFromPersistentStorage != SUCCESS)
                        {
                            msgSource = GENERAL;
                        }
                    }

                    if(rebootPending == 1)
                    {
                        msgSource = GENERAL;
                    }
                }
            }

            if(1)
            {
                if(1)
                {
                    removeExpiredResultHandlers(c);
                    removeExpiredOneToOneResponseHandlers(c);

                    {
                        static unsigned char businessLogicRunOnceAtStart = 0;

                        latestTick = getCurrentTick();

#if MEDIA_STREAMING_ENABLED == 1
                        if(mediaReplyReceived == 2)
                        {
                            if(latestTick >= (mediaMessageRequestTime + mediaReplyMessageWaitInterval))
                            {
                                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sMedia-Reply message not received in %u seconds ... rebooting"),
                                           MEDIA_ERROR, mediaReplyMessageWaitInterval);
                                error_log(LOG_GLOBAL_BUFFER);

                                exitApp(0);
                            }
                        }
#endif


                        /*
                         * Send network-stats if time has arrived.
                         */
                        if(latestTick >= nextSocketTick)
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Time to send network-stats !!!"));
                            info_log(LOG_GLOBAL_BUFFER);

                            sendClientData(get_network_data, TOPIC_NETWORK_DATA);

                            nextSocketTick = latestTick + NETWORK_INFO_INTERVAL;
                        }

                        /*
                         * Send PINGREQ, if time has arrived,
                         */
                        if((latestTick >= nextPingReqTick) && (pingRequestInterval != 0))
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Time to play ping-pong with server !!!\n"));
                            info_log(LOG_GLOBAL_BUFFER);

                            sendPingReqToServer(c);

                            nextPingReqTick = latestTick + pingRequestInterval;
                        }

#if SEND_GPS_LOCATION == 1
                        /*
                         * Send GPS-Location, if time has arrived,
                         */
                        if((latestTick >= nextSendGpsLocationTick) && (sendGpsLocationInterval != 0))
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Time to send GPS-location to server !!!\n"));
                            info_log(LOG_GLOBAL_BUFFER);

                            sendGpsLocationToServer();

                            nextSendGpsLocationTick = latestTick + sendGpsLocationInterval;
                        }
#endif

                        if((latestTick >= nextStatsDisplayTick) && (statsDisplayInterval != 0))
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER, "\n\n===== Bytes SENT = [%u], RECEIVED = [%u] =====\n\n",
                                       bytes_sent_over_wire, bytes_received_over_wire);
                            info_log(LOG_GLOBAL_BUFFER);

                            nextStatsDisplayTick = latestTick + statsDisplayInterval;
                        }

                        /*
                         * Time to run the business-logic !!
                         */
                        if((latestTick >= nextBusinessLogicTick) || (runBusinessLogicImmediately == 1) ||
                           (businessLogicRunOnceAtStart == 0))
                        {
                            if(coreLoopyBusinessLogicInitiatedBySelf != NULL)

                            {

#if ENSURE_EXPLICIT_TIME_SYNC == 1
                                if(timeSyncedViaExternalResources == 0)
                                {
                                    sg_sprintf(LOG_GLOBAL_BUFFER, "Time is still not synced, nothing to live for ...");
                                    error_log(LOG_GLOBAL_BUFFER);

                                    resetDevice();
                                }
#endif
                                coreLoopyBusinessLogicInitiatedBySelf();

#if SEND_GPS_LOCATION == 1
                                /*
                                 * Also, send the gps-location immediately, if this is the first time business-logic is being run.
                                 */
                                if((businessLogicRunOnceAtStart == 0) && (sendGpsLocationInterval != 0))
                                {
                                    sg_sprintf(LOG_GLOBAL_BUFFER, "Sending GPS-Location info for the first time ..");
                                    info_log(LOG_GLOBAL_BUFFER);

                                    sendGpsLocationToServer();
                                }
#endif
                                runBusinessLogicImmediately = 0;

                                if(businessLogicRunOnceAtStart == 0)
                                {
                                    msgSource = PERSISTENT_STORAGE;
                                }


                                nextBusinessLogicTick = latestTick + editableBusinessLogicInterval;
                                if((rebootPending == 1) && (businessLogicRunOnceAtStart == 1))
                                {
                                    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Rebooting due to an error condition that occurred before."));
                                    error_log(LOG_GLOBAL_BUFFER);

                                    exitApp(1);
                                }

                                businessLogicRunOnceAtStart = 1;
				                countdownTimerForBusinessLogic = 2 * editableBusinessLogicInterval;
                            }
                        }

#if MEDIA_STREAMING_ENABLED == 1
                        if(mediaStreamingErrorOccurred == 1)
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER,
                                       PROSTR("%sError occurred in media-streaming ... rebooting device to reset everything"), MEDIA);
                            error_log(LOG_GLOBAL_BUFFER);

                            exitApp(0);
                        }
#endif
                    }
                }

                /* This is 1 means physical-socket is fine, AND connection to InstaMsg-Server is fine at protocol level. */
                if(c->connected == 1)
                {
                }
                else if((c->ipstack).socketCorrupted == 0)
                {
                    static int connectionAttempts = 0;
                    connectionAttempts++;

                    sg_sprintf(LOG_GLOBAL_BUFFER,
                              PROSTR("Socket is fine at physical layer, but no connection established (yet) with InstaMsg-Server."));
                    error_log(LOG_GLOBAL_BUFFER);

                    if(connectionAttempts > MAX_CONN_ATTEMPTS_WITH_PHYSICAL_LAYER_FINE)
                    {
                        connectionAttempts = 0;

                        sg_sprintf(LOG_GLOBAL_BUFFER,
                                  PROSTR("Connection-Attempts exhausted ... so trying with re-initializing the socket-physical layer."));
                        error_log(LOG_GLOBAL_BUFFER);

                        (c->ipstack).socketCorrupted = 1;
                    }
                }

                if((c->ipstack).socketCorrupted == 1)
                {
                    clearInstaMsg(&instaMsg);
                    break;
                }
            }
        }
    }
}

