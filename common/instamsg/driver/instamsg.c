/*******************************************************************************
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

#include <string.h>

#define EMPTY_CLIENT_ID             PROSTR("EMPTY")
#define PROVISIONING_CLIENT_ID      PROSTR("PROVISIONING")

#define PROVISIONED                 PROSTR("PROVISIONED")
#define CONNECTED                   PROSTR("CONNECTED")


#define MAX_CYCLES_TO_WAIT_FOR_PUBACK   10
int pubAckMsgId;
int pubAckRecvAttempts;
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

#define SECRET PROSTR("SECRET")

#if MEDIA_STREAMING_ENABLED == 1
#include "./include/media.h"

static unsigned char mediaReplyReceived;
unsigned long mediaMessageRequestTime;

static char streamId[MAX_BUFFER_SIZE];
#endif

static int editableBusinessLogicInterval;
static unsigned char mqttConnectFlag;

static unsigned char notifyServerOfSecretReceived;

#define DATA_LOG_TOPIC      PROSTR("topic")
#define DATA_LOG_PAYLOAD    PROSTR("payload")


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
    if(clientId)
        sg_free(clientId);

    if(logging)
        sg_free(logging);

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
    if(lastPubTopic)
    {
        sg_free(lastPubTopic);
        lastPubTopic = NULL;
    }

    if(lastPubPayload)
    {
        sg_free(lastPubPayload);
        lastPubPayload = NULL;
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
    if(peerMessage)
        sg_free(peerMessage);

    if(peer)
        sg_free(peer);
}


static void handleConfigReceived(InstaMsg *c, MQTTMessage *msg)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sReceived the config-payload [%s] from server"), CONFIG, (char*)(msg->payload));
    info_log(LOG_GLOBAL_BUFFER);

    process_config(msg->payload);
}


static int sendPacket(InstaMsg *c, unsigned char *buf, int length)
{
    int rc = SUCCESS;

    /*
     * We assume that if a packet cannot be sent within 30 seconds,
     * there has been some (undetectable) issue somewehre.
     */
    watchdog_reset_and_enable(60, "sendPacket", NULL, NULL);

    if((c->ipstack).socketCorrupted == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Socket not available at physical layer .. so packet cannot be sent to server."));
        error_log(LOG_GLOBAL_BUFFER);

        rc = FAILURE;
        goto exit;
    }

    if(mqttConnectFlag != 1)
    {
        if(c->connected == 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("No CONNACK received from server .. so packet cannot be sent to server."));
            error_log(LOG_GLOBAL_BUFFER);

            rc = FAILURE;
            goto exit;
        }
    }
    mqttConnectFlag = 0;

    if((c->ipstack).write(&(c->ipstack), buf, length) == FAILURE)
    {
        (c->ipstack).socketCorrupted = 1;
        rc = FAILURE;
    }

exit:
    watchdog_disable();
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
                              "readPacket", NULL, NULL);

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
    watchdog_disable();
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
            rc = sendPacket(c, GLOBAL_BUFFER, len);
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
        if(replyTopic)
            sg_free(replyTopic);

        if(messageId)
            sg_free(messageId);

        if(method)
            sg_free(method);
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


static void handleFileTransfer(InstaMsg *c, MQTTMessage *msg)
{
    const char *REPLY_TOPIC = PROSTR("reply_to");
    const char *MESSAGE_ID = PROSTR("message_id");
    const char *METHOD = PROSTR("method");
    char *replyTopic, *messageId, *method, *url, *filename, *ackMessage;

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

#if FILE_SYSTEM_ENABLED == 1
        HTTPResponse response = downloadFile(url, filename, NULL, NULL, 10);
        if(response.status == HTTP_FILE_DOWNLOAD_SUCCESS)
        {
            ackStatus = 1;
        }
        sg_sprintf(ackMessage, PROSTR("{\"response_id\": \"%s\", \"status\": %d}"), messageId, ackStatus);
#else
        sg_sprintf(ackMessage, PROSTR("{\"response_id\": \"%s\", \"status\": %d}"), messageId, ackStatus);
#endif

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

        sg_sprintf(clientIdBuf, "%s-%s", c->connectOptions.clientID.cstring, c->connectOptions.username.cstring);
        headers[1].value = clientIdBuf;

        headers[2].key = "Content-Type";
        headers[2].value = "multipart/form-data; boundary=" POST_BOUNDARY;

        headers[3].key = CONTENT_LENGTH;
        headers[3].value = "0"; /* This will be updated to proper bytes later. */

        headers[4].key = 0;
        headers[4].value = 0;


        response = uploadFile(c->fileUploadUrl, filename, NULL, headers, 10);

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
            0);

exit:
    if(replyTopic)
        sg_free(replyTopic);

    if(messageId)
        sg_free(messageId);

    if(method)
        sg_free(method);

    if(url)
        sg_free(url);

    if(filename)
        sg_free(filename);

    if(ackMessage)
        sg_free(ackMessage);


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
        sendPacket(c, GLOBAL_BUFFER, len);
    }
}


void clearInstaMsg(InstaMsg *c)
{
    release_socket(&(c->ipstack));

#if FILE_SYSTEM_ENABLED == 1
    release_file_system(&(c->singletonUtilityFs));
#endif

    c->connected = 0;
}


static void setValuesOfSpecialTopics(InstaMsg *c)
{
    memset(c->filesTopic, 0, sizeof(c->filesTopic));
    sg_sprintf(c->filesTopic, PROSTR("instamsg/clients/%s/files"), c->clientIdComplete);

    memset(c->rebootTopic, 0, sizeof(c->rebootTopic));
    sg_sprintf(c->rebootTopic, PROSTR("instamsg/clients/%s/reboot"), c->clientIdComplete);

    memset(c->enableServerLoggingTopic, 0, sizeof(c->enableServerLoggingTopic));
    sg_sprintf(c->enableServerLoggingTopic, PROSTR("instamsg/clients/%s/enableServerLogging"), c->clientIdComplete);

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

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("\n\nThe special-topics value :: \n\n"
             "\r\nFILES_TOPIC = [%s],"
             "\r\nREBOOT_TOPIC = [%s],"
             "\r\nENABLE_SERVER_LOGGING_TOPIC = [%s],"
             "\r\nSERVER_LOGS_TOPIC = [%s],"
             "\r\nFILE_UPLOAD_URL = [%s],"
             "\r\nCONFIG_FROM_SERVER_TO_CLIENT = [%s]"),
              c->filesTopic, c->rebootTopic, c->enableServerLoggingTopic,
              c->serverLogsTopic, c->fileUploadUrl, c->receiveConfigTopic);
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
}


void initInstaMsg(InstaMsg* c,
                  int (*connectHandler)(),
                  int (*disconnectHandler)(),
                  int (*oneToOneMessageHandler)())
{
    int i;

    runBusinessLogicImmediately = 0;

    c->FRESH_SERVER_LOGS_TIME = -1;

#if FILE_SYSTEM_ENABLED == 1
    init_file_system(&(c->singletonUtilityFs), "");
#endif


    (c->ipstack).socketCorrupted = 1;
	init_socket(&(c->ipstack), INSTAMSG_HOST, INSTAMSG_PORT);
    if((c->ipstack).socketCorrupted ==1)
    {
        return;
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

    init_data_logger();
    pubAckRecvAttempts = 0;
    waitingForPuback = NOT_WAITING_FOR_PUBACK;
    msgSource = GENERAL;
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
            if(topic)
                sg_free(topic);

            if(payload)
                sg_free(payload);
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
    if(connack_rc == 0x00)  /* Connection Accepted */
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%s successfully to InstaMsg-Server."), mode);
        info_log(LOG_GLOBAL_BUFFER);

        c->connected = 1;

        sendClientData(get_client_session_data, TOPIC_SESSION_DATA);
        sendClientData(get_client_metadata, TOPIC_METADATA);
        sendClientData(get_network_data, TOPIC_NETWORK_DATA);

        registerEditableConfig(&pingRequestInterval,
                               PROSTR("PING_REQ_INTERVAL"),
                               CONFIG_INT,
                               PROSTR("180"),
                               PROSTR("Keep-Alive Interval between Device and InstaMsg-Server"));

        registerEditableConfig(&compulsorySocketReadAfterMQTTPublishInterval,
                               PROSTR("COMPULSORY_SOCKET_READ_AFTER_MQTT_PUBLISH_INTERVAL"),
                               CONFIG_INT,
                               "3",
                               PROSTR("This variable controls after how many MQTT-Publishes a compulsory socket-read is done. This prevents any socket-pverrun errors (particularly in hardcore embedded-devices"));

        {
            char interval[6];
            memset(interval, 0, sizeof(interval));

            sg_sprintf(interval, "%d", editableBusinessLogicInterval);
            registerEditableConfig(&editableBusinessLogicInterval,
                                   PROSTR("BUSINESS_LOGIC_INTERVAL"),
                                   CONFIG_INT,
                                   interval,
                                   PROSTR("Business-Logic Interval (in seconds)"));
        }

#if MEDIA_STREAMING_ENABLED == 1
        registerEditableConfig(&mediaStreamingEnabledRuntime,
                               PROSTR("MEDIA_STREAMING_ENABLED"),
                               CONFIG_INT,
                               "0",
                               PROSTR("0 - Disabled; 1 - Enabled"));
        if(mediaStreamingEnabledRuntime == 1)
        {
            initiateStreaming();
        }
#endif

        if(notifyServerOfSecretReceived == 1)
        {
            mqttConnectFlag = 1;
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
        info_log(LOG_GLOBAL_BUFFER);
    }
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
                    if(connack_rc == 0x00)  /* Connection Accepted */
                    {
                        memcpy(c->clientIdComplete, msg.payload, msg.payloadlen);

                        {
                            int remainingPayloadLength = msg.payloadlen - 36;
                            if(remainingPayloadLength > 1)
                            {
                                memset(c->password, 0, sizeof(c->password));
                                memcpy(c->password, (char*)msg.payload + 37, remainingPayloadLength - 1);

                                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Received client-secret from server via PROVACK [%s]"), c->password);
                                info_log(LOG_GLOBAL_BUFFER);

                                memset(messageBuffer, 0, sizeof(messageBuffer));
                                generate_config_json(messageBuffer, SECRET, CONFIG_STRING, c->clientIdComplete, "");
                                save_config_value_on_persistent_storage(SECRET, messageBuffer);

                                /*
                                 * Send notification to the server, that the secret-password has been saved.
                                 */
                                notifyServerOfSecretReceived = 1;
                           }
                        }

                        memset(c->clientIdComplete, 0, sizeof(c->clientIdComplete));
                        memcpy(c->clientIdComplete, msg.payload, 36);
                        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Received client-id from server via PROVACK [%s]"), c->clientIdComplete);
                        info_log(LOG_GLOBAL_BUFFER);

                        setValuesOfSpecialTopics(c);
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

                        rebootDevice();
                    }
                    else if(strcmp(topicName, c->clientIdComplete) == 0)
                    {
                        oneToOneMessageArrived(c, &msg);
                    }
                    else if(strcmp(topicName, c->receiveConfigTopic) == 0)
                    {
                        handleConfigReceived(c, &msg);
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
                sendPacket(c, GLOBAL_BUFFER, len); /* send the PUBREL packet */

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

    mqttConnectFlag = 1;
    sendPacket(c, GLOBAL_BUFFER, len);

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
                return;
            }

            memset(messageSavingJson, 0, messageJsonSize);
            sg_sprintf(messageSavingJson, "{'%s' : '%s', '%s' : '%s'}",
                       DATA_LOG_TOPIC, lastPubTopic,
                       DATA_LOG_PAYLOAD, lastPubPayload);

            save_record_to_persistent_storage(messageSavingJson);
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

    if ((rc = sendPacket(c, GLOBAL_BUFFER, len)) != SUCCESS) /* send the subscribe packet */
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
    if ((rc = sendPacket(c, GLOBAL_BUFFER, len)) != SUCCESS) /* send the subscribe packet */
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
    if ((rc = sendPacket(c, GLOBAL_BUFFER, len)) != SUCCESS) /* send the subscribe packet */
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
    return publish(topic,
                   payload,
                   QOS1,
                   0,
                   publishAckReceived,
                   MQTT_RESULT_HANDLER_TIMEOUT,
                   1);
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
        rc = sendPacket(c, GLOBAL_BUFFER, len);            /* send the disconnect packet */

    if(c->onDisconnectCallback != NULL)
    {
        c->onDisconnectCallback();
    }

    return rc;
}


void start(int (*onConnectOneTimeOperations)(),
           int (*onDisconnect)(),
           int (*oneToOneMessageReceivedHandler)(OneToOneResult *),
           void (*coreLoopyBusinessLogicInitiatedBySelf)(),
           int businessLogicInterval)
{
    InstaMsg *c = &instaMsg;

    volatile unsigned long latestTick = getCurrentTick();
    unsigned long nextSocketTick = latestTick + NETWORK_INFO_INTERVAL;
    unsigned long nextPingReqTick = latestTick + pingRequestInterval;

    unsigned long nextBusinessLogicTick;
    editableBusinessLogicInterval = businessLogicInterval;
    nextBusinessLogicTick = latestTick + editableBusinessLogicInterval;


    mqttConnectFlag = 0;
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
                            if(latestTick >= (mediaMessageRequestTime + 10))
                            {
                                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sMedia-Reply message not received in 10 seconds ... rebooting"),
                                           MEDIA_ERROR);
                                error_log(LOG_GLOBAL_BUFFER);

                                rebootDevice();
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

                        /*
                         * Time to run the business-logic !!
                         */
                        if((latestTick >= nextBusinessLogicTick) || (runBusinessLogicImmediately == 1) ||
                           (businessLogicRunOnceAtStart == 0))
                        {
                            if((coreLoopyBusinessLogicInitiatedBySelf != NULL) && (msgSource == GENERAL))

                            {
                                coreLoopyBusinessLogicInitiatedBySelf(NULL);
                                runBusinessLogicImmediately = 0;

                                if(businessLogicRunOnceAtStart == 0)
                                {
                                    msgSource = PERSISTENT_STORAGE;
                                }

                                if((rebootPending == 1) && (businessLogicRunOnceAtStart == 1))
                                {
                                    sg_sprintf(LOG_GLOBAL_BUFFER,
                                               PROSTR(
                                                  "There were some messages which did not complete send-cum-ack cycle, so rebooting"));
                                    error_log(LOG_GLOBAL_BUFFER);

                                    rebootDevice();
                                }

                                businessLogicRunOnceAtStart = 1;
                                nextBusinessLogicTick = latestTick + editableBusinessLogicInterval;
                            }

                        }

#if MEDIA_STREAMING_ENABLED == 1
                        if(mediaStreamingErrorOccurred == 1)
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER,
                                       PROSTR("%sError occurred in media-streaming ... rebooting device to reset everything"), MEDIA);
                            error_log(LOG_GLOBAL_BUFFER);

                            rebootDevice();
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

