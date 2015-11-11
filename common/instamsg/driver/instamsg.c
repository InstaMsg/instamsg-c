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

#include <string.h>

#define NO_CLIENT_ID "NONE"

static void publishAckReceived(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    info_log("[DEFAULT-PUBLISH-HANDLER] PUBACK received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
}


static void serverLoggingTopicMessageArrived(InstaMsg *c, MQTTMessage *msg)
{
    /*
     * The payload is of the format ::
     *              {'client_id':'cc366750-e286-11e4-ace1-bc764e102b63','logging':1}
     */

    const char *CLIENT_ID = "client_id";
    const char *LOGGING = "logging";
    char *clientId, *logging;

    clientId = (char *)sg_malloc(MAX_BUFFER_SIZE);
    logging = (char *)sg_malloc(MAX_BUFFER_SIZE);
    if((clientId == NULL) || (logging == NULL))
    {
        error_log("Could not allocate memory in serverLoggingTopicMessageArrived");
        goto exit;
    }

    getJsonKeyValueIfPresent(msg->payload, CLIENT_ID, clientId);
    getJsonKeyValueIfPresent(msg->payload, LOGGING, logging);

    if( (strlen(clientId) > 0) && (strlen(logging) > 0) )
    {
        if(strcmp(logging, "1") == 0)
        {
            c->serverLoggingEnabled = 1;
            info_log(SERVER_LOGGING "Enabled.");
        }
        else
        {
            c->serverLoggingEnabled = 0;
            info_log(SERVER_LOGGING "Disabled.");
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
    info_log("PUBCOMP received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
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


static void fireResultHandlerAndRemove(InstaMsg *c, MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    int i;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->resultHandlers[i].msgId == fixedHeaderPlusMsgId->msgId)
        {
            c->resultHandlers[i].fp(fixedHeaderPlusMsgId);
            c->resultHandlers[i].msgId = 0;

            break;
        }
    }
}


static void attachOneToOneResponseHandler(InstaMsg *c,
                                          int msgId,
                                          unsigned int timeout,
                                          void (*oneToOneResponseHandler)(OneToOneResult *))
{
    int i;

    if(oneToOneResponseHandler == NULL)
    {
        return;
    }


    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->oneToOneResponseHandlers[i].msgId == 0)
        {
            c->oneToOneResponseHandlers[i].msgId = msgId;
            c->oneToOneResponseHandlers[i].timeout = timeout;
            c->oneToOneResponseHandlers[i].fp = oneToOneResponseHandler;

            break;
        }
    }
}


static int fireOneToOneResponseHandlerAndRemove(InstaMsg *c, int msgId, OneToOneResult *oneToOneResult)
{
    int i;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->oneToOneResponseHandlers[i].msgId == msgId)
        {
            c->oneToOneResponseHandlers[i].fp(oneToOneResult);
            c->oneToOneResponseHandlers[i].msgId = 0;

            return SUCCESS;
        }
    }

    return FAILURE;
}


static int doMqttSendPublish(const char *topic, const char *message)
{
    return MQTTPublish(topic,
                       message,
                       QOS2,
                       0,
                       publishAckReceived,
                       MQTT_RESULT_HANDLER_TIMEOUT,
                       0,
                       1);
}


static void MQTTReplyOneToOne(OneToOneResult *oneToOneResult, const char *replyMessage)
{
    InstaMsg *c = &instaMsg;
    int id = getNextPacketId(c);

    memset(messageBuffer, 0, sizeof(messageBuffer));
    sg_sprintf(messageBuffer, "{\"message_id\": \"%u\", \"response_id\": \"%u\", \"reply_to\": \"%s\", \"body\": \"%s\", \"status\": 1}",
               id,
               oneToOneResult->peerMsgId,
               c->clientIdComplete,
               replyMessage);

    doMqttSendPublish(oneToOneResult->peer, messageBuffer);
}


static void oneToOneMessageArrived(InstaMsg *c, MQTTMessage *msg)
{
    char *peerMessage = NULL, *peer = NULL;
    char peerMsgId[6];
    char responseMsgId[6];

    info_log(ONE_TO_ONE " Payload == [%s]", msg->payload);

    peerMessage = (char*) sg_malloc(MAX_BUFFER_SIZE);
    if(peerMessage == NULL)
    {
        error_log(ONE_TO_ONE "Could not allocate memory for message received from peer");
        goto exit;
    }
    getJsonKeyValueIfPresent(msg->payload, "body", peerMessage);

    peer = (char*) sg_malloc(50);
    if(peer == NULL)
    {
        error_log(ONE_TO_ONE "Could not allocate memory for peer-value");
        goto exit;
    }
    getJsonKeyValueIfPresent(msg->payload, "reply_to", peer);

    memset(peerMsgId, 0, sizeof(peerMsgId));
    getJsonKeyValueIfPresent(msg->payload, "message_id", peerMsgId);

    memset(responseMsgId, 0, sizeof(responseMsgId));
    getJsonKeyValueIfPresent(msg->payload, "response_id", responseMsgId);

    if(strlen(peerMsgId) == 0)
    {
        error_log(ONE_TO_ONE "Peer-Message-Id not received ... not proceeding further");
        goto exit;
    }
    if(strlen(peer) == 0)
    {
        error_log(ONE_TO_ONE "Peer-value not received ... not proceeding further");
        goto exit;
    }


    {
        OneToOneResult oneToOneResult;

        oneToOneResult.succeeded = 1;
        oneToOneResult.peerMsg = peerMessage;
        oneToOneResult.peer = peer;
        oneToOneResult.peerMsgId = sg_atoi(peerMsgId);
        oneToOneResult.reply = &MQTTReplyOneToOne;

        debug_log("Peer-Message = [%s], Peer = [%s], Peer-Message-Id = [%u]",
                   oneToOneResult.peerMsg,
                   oneToOneResult.peer,
                   oneToOneResult.peerMsgId);

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
            if(fireOneToOneResponseHandlerAndRemove(c, sg_atoi(responseMsgId), &oneToOneResult) == FAILURE)
            {
                error_log(ONE_TO_ONE "No handler found for one-to-one for message-id [%s]", responseMsgId);
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
    info_log(CONFIG "Received the config-payload [%s] from server", msg->payload);
    process_config(msg->payload);
}


static int sendPacket(InstaMsg *c, unsigned char *buf, int length)
{
    int rc = SUCCESS;

    /*
     * We assume that if a packet cannot be sent within 30 seconds,
     * there has been some (undetectable) issue somewehre.
     */
    watchdog_reset_and_enable(30, "sendPacket");

    if((c->ipstack).socketCorrupted == 1)
    {
        error_log("Socket not available at physical layer .. so packet cannot be sent to server.");

        rc = FAILURE;
        goto exit;
    }

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

    watchdog_reset_and_enable(10 * MAX_TRIES_ALLOWED_WHILE_READING_FROM_SOCKET_MEDIUM * SOCKET_READ_TIMEOUT_SECS,
                              "readPacket");

    if((c->ipstack).socketCorrupted == 1)
    {
        error_log("Socket not available at physical layer .. so packet cannot be read from server.");
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
    } while((rc == SOCKET_READ_TIMEOUT) && (numRetries >= 0));

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


static void logJsonFailureMessageAndReturn(const char *key, MQTTMessage *msg)
{
    error_log(FILE_TRANSFER "Could not find key [%s] in message-payload [%s] .. not proceeding further", key, msg->payload);
}


static void handleFileTransfer(InstaMsg *c, MQTTMessage *msg)
{
    const char *REPLY_TOPIC = "reply_to";
    const char *MESSAGE_ID = "message_id";
    const char *METHOD = "method";
    char *replyTopic, *messageId, *method, *url, *filename, *ackMessage;

    replyTopic = (char *)sg_malloc(MAX_BUFFER_SIZE);
    messageId = (char *)sg_malloc(50);
    method = (char *)sg_malloc(20);
    url = (char *)sg_malloc(MAX_BUFFER_SIZE);
    filename = (char *)sg_malloc(MAX_BUFFER_SIZE);
    ackMessage = (char *)sg_malloc(MAX_BUFFER_SIZE);

    getJsonKeyValueIfPresent(msg->payload, REPLY_TOPIC, replyTopic);
    getJsonKeyValueIfPresent(msg->payload, MESSAGE_ID, messageId);
    getJsonKeyValueIfPresent(msg->payload, METHOD, method);
    getJsonKeyValueIfPresent(msg->payload, "url", url);
    getJsonKeyValueIfPresent(msg->payload, "filename", filename);

    if(strlen(replyTopic) == 0)
    {
        logJsonFailureMessageAndReturn(REPLY_TOPIC, msg);
        goto exit;
    }
    if(strlen(messageId) == 0)
    {
        logJsonFailureMessageAndReturn(MESSAGE_ID, msg);
        goto exit;
    }
    if(strlen(method) == 0)
    {
        logJsonFailureMessageAndReturn(METHOD, msg);
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
        HTTPResponse response = {0};

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
        response = downloadFile(url, filename, NULL, NULL, 10);
#endif
        if(response.status == HTTP_FILE_DOWNLOAD_SUCCESS)
        {
            ackStatus = 1;
        }
        sg_sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": %d}", messageId, ackStatus);

    }
    else if( (strcmp(method, "GET") == 0) && (strlen(filename) == 0))
    {
        char *fileListing;

        RESET_GLOBAL_BUFFER;
        fileListing = (char*)GLOBAL_BUFFER;

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
        (c->singletonUtilityFs).getFileListing(&(c->singletonUtilityFs), fileListing, MAX_BUFFER_SIZE, ".");
#else
        strcat(fileListing, "{}");
#endif

        info_log(FILE_LISTING ": [%s]", fileListing);
        sg_sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": 1, \"files\": %s}", messageId, fileListing);
    }
    else if( (strcmp(method, "DELETE") == 0) && (strlen(filename) > 0))
    {
        int status = FAILURE;

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
        status = (c->singletonUtilityFs).deleteFile(&(c->singletonUtilityFs), filename);
#endif

        if(status == SUCCESS)
        {
            info_log(FILE_DELETE "[%s] deleted successfully.", filename);
            sg_sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": 1}", messageId);
        }
        else
        {
            error_log(FILE_DELETE "[%s] could not be deleted :(", filename);
            sg_sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": 0, \"error_msg\":\"%s\"}", messageId, "File-Removal Failed :(");
        }
    }
    else if( (strcmp(method, "GET") == 0) && (strlen(filename) > 0))
    {
        HTTPResponse response = {0};

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
        char *clientIdBuf;
        KeyValuePairs headers[5];

        headers[0].key = "Authorization";
        headers[0].value = c->connectOptions.password.cstring;

        headers[1].key = "ClientId";

        clientIdBuf = (char*) sg_malloc(MAX_BUFFER_SIZE);
        if(clientIdBuf == NULL)
        {
            error_log(FILE_UPLOAD "Failed to allocate memory");
            goto terminateFileUpload;
        }

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
#endif
        if(response.status == HTTP_FILE_UPLOAD_SUCCESS)
        {
            sg_sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": 1, \"url\": \"%s\"}", messageId, response.body);
        }
        else
        {
            sg_sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": 0}", messageId);
        }
    }


    /*
     * Send the acknowledgement, along with the ackStatus (success/failure).
     */
    MQTTPublish(replyTopic,
                ackMessage,
                (msg->fixedHeaderPlusMsgId).fixedHeader.qos,
                (msg->fixedHeaderPlusMsgId).fixedHeader.dup,
                NULL,
                MQTT_RESULT_HANDLER_TIMEOUT,
                0,
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
        info_log("No %s received for msgid [%u], removing..", info, *msgId);
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
        checkAndRemoveExpiredHandler(&(c->oneToOneResponseHandlers[i].msgId), &(c->oneToOneResponseHandlers[i].timeout), "one-to-one response");
    }
}


void sendPingReqToServer(InstaMsg *c)
{
    int len;

    RESET_GLOBAL_BUFFER;
    len = MQTTSerialize_pingreq(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));

    if((c->ipstack).socketCorrupted == 1)
    {
        error_log("Socket not available at physical layer .. so server cannot be pinged for maintaining keep-alive.");
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

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
    release_file_system(&(c->singletonUtilityFs));
#endif

    c->connected = 0;
}


static void setValuesOfSpecialTopics(InstaMsg *c)
{
    memset(c->filesTopic, 0, sizeof(c->filesTopic));
    sg_sprintf(c->filesTopic, "instamsg/clients/%s/files", c->clientIdComplete);

    memset(c->rebootTopic, 0, sizeof(c->rebootTopic));
    sg_sprintf(c->rebootTopic, "instamsg/clients/%s/reboot", c->clientIdComplete);

    memset(c->enableServerLoggingTopic, 0, sizeof(c->enableServerLoggingTopic));
    sg_sprintf(c->enableServerLoggingTopic, "instamsg/clients/%s/enableServerLogging", c->clientIdComplete);

    memset(c->serverLogsTopic, 0, sizeof(c->serverLogsTopic));
    sg_sprintf(c->serverLogsTopic, "instamsg/clients/%s/logs", c->clientIdComplete);

    memset(c->fileUploadUrl, 0, sizeof(c->fileUploadUrl));
    sg_sprintf(c->fileUploadUrl, "/api/beta/clients/%s/files", c->clientIdComplete);

    memset(c->receiveConfigTopic, 0, sizeof(c->receiveConfigTopic));
    sg_sprintf(c->receiveConfigTopic, "instamsg/clients/%s/config/serverToClient", c->clientIdComplete);



    info_log("\n\nThe special-topics value :: \n\n"
             "\r\nFILES_TOPIC = [%s],"
             "\r\nREBOOT_TOPIC = [%s],"
             "\r\nENABLE_SERVER_LOGGING_TOPIC = [%s],"
             "\r\nSERVER_LOGS_TOPIC = [%s],"
             "\r\nFILE_UPLOAD_URL = [%s],"
             "\r\nCONFIG_FROM_SERVER_TO_CLIENT = [%s]\n",
             c->filesTopic, c->rebootTopic, c->enableServerLoggingTopic,
             c->serverLogsTopic, c->fileUploadUrl, c->receiveConfigTopic);
}


void initInstaMsg(InstaMsg* c,
                  int (*connectHandler)(),
                  int (*disconnectHandler)(),
                  int (*oneToOneMessageHandler)())
{
    int i;


    init_config();

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
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

        c->oneToOneResponseHandlers[i].msgId = 0;
        c->oneToOneResponseHandlers[i].timeout = 0;
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

    memset(c->clientIdMachine, 0, sizeof(c->clientIdMachine));
    strcpy(c->clientIdMachine, NO_CLIENT_ID);
    c->connectOptions.clientID.cstring = c->clientIdMachine;

    memset(c->username, 0, sizeof(c->username));
    strcpy(c->username, "");
    c->connectOptions.username.cstring = c->username;

    memset(c->password, 0, sizeof(c->password));
    get_device_uuid(c->password, sizeof(c->password));
    c->connectOptions.password.cstring = c->password;

    c->connected = 0;
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
        MQTTPublish(topicName,
                    messageBuffer,
                    QOS1,
                    0,
                    NULL,
                    MQTT_RESULT_HANDLER_TIMEOUT,
                    0,
                    1);
    }
    else
    {
        info_log("Not publishing empty-message to topic [%s]", topicName);
    }
}


static void handleConnOrProvAckGeneric(InstaMsg *c, int connack_rc)
{
    if(connack_rc == 0x00)  /* Connection Accepted */
    {
        info_log("Connected successfully to InstaMsg-Server.");
        c->connected = 1;

        sendClientData(get_client_session_data, TOPIC_SESSION_DATA);
        sendClientData(get_client_metadata, TOPIC_METADATA);
        sendClientData(get_network_data, TOPIC_NETWORK_DATA);

        add_editable_config(&pingRequestInterval,
                            "PING_REQ_INTERVAL",
                            CONFIG_INT,
                            "180",
                            "Keep-Alive Interval between Device and InstaMsg-Server");

        add_editable_config(&compulsorySocketReadAfterMQTTPublishInterval,
                            "COMPULSORY_SOCKET_READ_AFTER_MQTT_PUBLISH_INTERVAL",
                            CONFIG_INT,
                            "3",
                            "This variable controls after how many MQTT-Publishes a compulsory socket-read is done. This prevents any socket-pverrun errors (particularly in hardcore embedded-devices");

        if(c->onConnectCallback != NULL)
        {
            c->onConnectCallback();
            c->onConnectCallback = NULL;
        }
    }
    else
    {
        info_log("Client-Connection failed with code [%d]", connack_rc);
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
                    handleConnOrProvAckGeneric(c, connack_rc);
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
                    handleConnOrProvAckGeneric(c, connack_rc);
                    if(connack_rc == 0x00)  /* Connection Accepted */
                    {
                        memcpy(c->clientIdComplete, msg.payload, msg.payloadlen);
                        info_log("Received client-id from server via PROVACK [%s]", c->clientIdComplete);

                        setValuesOfSpecialTopics(c);
                    }
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
                        error_log("Could not allocate memory for topic");
                        goto publish_exit;
                    }
                    else
                    {
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
                info_log("PINGRESP received... relations are intact !!\n");
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
    InstaMsg *c = (InstaMsg *)arg;

    RESET_GLOBAL_BUFFER;
    if ((len = MQTTSerialize_connect(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), &(c->connectOptions))) <= 0)
    {
        return NULL;
    }

    sendPacket(c, GLOBAL_BUFFER, len);

    return NULL;
}


int MQTTSubscribe(const char* topicName,
                  const enum QoS qos,
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
		info_log("Subscribing to topic [%s]", topicName);
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
            info_log("Subscribed successfully.\n");
        }
        else
        {
            info_log("Subscribing failed, error-code = [%d]\n", rc);
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


int MQTTPublish(const char* topicName,
                const char* payload,
                const enum QoS qos,
                const char dup,
                void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                unsigned int resultHandlerTimeout,
                const char retain,
                const char logging)
{
    static unsigned int publishCount = 0;
    int rc = FAILURE;
    int len = 0;
    int id = -1;
    InstaMsg *c = &instaMsg;

    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;

    RESET_GLOBAL_BUFFER;

    if (qos == QOS1 || qos == QOS2)
    {
        id = getNextPacketId(c);

        /*
         * We will get PUBACK from server only for QOS1 and QOS2.
         * So, it makes sense to lodge the result-handler only for these cases.
         */
        attachResultHandler(c, id, resultHandlerTimeout, resultHandler);
    }

    if(logging == 1)
    {
		info_log("Publishing message [%s] to topic [%s]", payload, topicName);
    }

    len = MQTTSerialize_publish(GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER), 0, qos, retain, id, topic, (unsigned char*)payload, strlen((char*)payload) + 1);
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

    if(logging == 1)
    {
        if(rc == SUCCESS)
        {
            info_log("Published successfully.\n");

            if(compulsorySocketReadAfterMQTTPublishInterval != 0)
            {
                if((publishCount % compulsorySocketReadAfterMQTTPublishInterval) == 0)
                {
                    info_log("Doing out-of-order socket-read, as [%u] MQTT-Publishes have been done",
                             compulsorySocketReadAfterMQTTPublishInterval);

                    readAndProcessIncomingMQTTPacketsIfAny(c);
                }
            }
        }
        else
        {
            info_log("Publishing failed, error-code = [%d]\n", rc);
        }
    }

    return rc;
}


int MQTTSend(const char* peer,
              const char* payload,
              void (*oneToOneResponseReceivedHandler)(OneToOneResult *),
              unsigned int timeout)
{
    InstaMsg *c = &instaMsg;
    int id = getNextPacketId(c);

    memset(messageBuffer, 0, sizeof(messageBuffer));
    sg_sprintf(messageBuffer, "{\"message_id\": \"%u\", \"reply_to\": \"%s\", \"body\": \"%s\"}", id, c->clientIdComplete, payload);

    attachOneToOneResponseHandler(&instaMsg, id, timeout, oneToOneResponseReceivedHandler);
    return doMqttSendPublish(peer, messageBuffer);
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
    unsigned long nextBusinessLogicTick = latestTick + businessLogicInterval;


    pingRequestInterval = 0;
    compulsorySocketReadAfterMQTTPublishInterval = 0;

    while(1)
    {
        initInstaMsg(c, onConnectOneTimeOperations, onDisconnect, oneToOneMessageReceivedHandler);

        RESET_GLOBAL_BUFFER;
        get_device_uuid((char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));
        info_log("Device-UUID :: [%s]", (char*)GLOBAL_BUFFER);

        while(1)
        {

            if((c->ipstack).socketCorrupted == 1)
            {
                error_log("Socket not available at physical layer .. so nothing can be read from socket.");
            }
            else
            {
                readAndProcessIncomingMQTTPacketsIfAny(c);
            }

            if(1)
            {
                /*
                 * We ensure that the business-logic is initiated only after the interval, and do the intermediate
                 * works as and when the time arrives.
                 */
                while(1)
                {
                    removeExpiredResultHandlers(c);
                    removeExpiredOneToOneResponseHandlers(c);

                    startAndCountdownTimer(1, 0);

                    {
                        latestTick = getCurrentTick();


                        /*
                         * Send network-stats if time has arrived.
                         */
                        if(latestTick >= nextSocketTick)
                        {
                            info_log("Time to send network-stats !!!");
                            sendClientData(get_network_data, TOPIC_NETWORK_DATA);

                            nextSocketTick = latestTick + NETWORK_INFO_INTERVAL;
                        }

                        /*
                         * Send PINGREQ, if time has arrived,
                         */
                        if(latestTick >= nextPingReqTick)
                        {
                            info_log("Time to play ping-pong with server !!!\n");
                            sendPingReqToServer(c);

                            nextPingReqTick = latestTick + pingRequestInterval;
                        }

                        /*
                         * Time to run the business-logic !!
                         */
                        if(latestTick >= nextBusinessLogicTick)
                        {
                            if(coreLoopyBusinessLogicInitiatedBySelf != NULL)
                            {
                                coreLoopyBusinessLogicInitiatedBySelf(NULL);
                            }

                            nextBusinessLogicTick = latestTick + businessLogicInterval;
                            break;
                        }
                    }
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

                error_log("Socket is fine at physical layer, but no connection established (yet) with InstaMsg-Server.");
                if(connectionAttempts > MAX_CONN_ATTEMPTS_WITH_PHYSICAL_LAYER_FINE)
                {
                    connectionAttempts = 0;
                    error_log("Connection-Attempts exhausted ... so trying with re-initializing the socket-physical layer.");

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

