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

#include "include/instamsg.h"
#include "include/httpclient.h"

#include <string.h>
#include <signal.h>

static void serverLoggingTopicMessageArrived(InstaMsg *c, MQTTMessage *msg)
{
    /*
     * The payload is of the format ::
     *              {'client_id':'cc366750-e286-11e4-ace1-bc764e102b63','logging':1}
     */

    const char *CLIENT_ID = "client_id";
    const char *LOGGING = "logging";

    char clientId[MAX_BUFFER_SIZE] = {0};
    char logging[MAX_BUFFER_SIZE] = {0};

    getJsonKeyValueIfPresent(msg->payload, CLIENT_ID, clientId);
    getJsonKeyValueIfPresent(msg->payload, LOGGING, logging);

    if( (strlen(clientId) > 0) && (strlen(logging) > 0) )
    {
        if(atoi(logging) == 1)
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
}


static void publishQoS2CycleCompleted(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    info_log("PUBCOMP received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
}


static void NewMessageData(MessageData* md, InstaMsg *c, MQTTString* aTopicName, MQTTMessage* aMessgage) {
    md->c = c;
    md->topicName = aTopicName;
    md->message = aMessgage;
}


static int getNextPacketId(InstaMsg *c) {
    int id = c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
    return id;
}


static void attachResultHandler(InstaMsg *c, int msgId, unsigned int timeout, void (*resultHandler)(MQTTFixedHeaderPlusMsgId *))
{
    if(resultHandler == NULL)
    {
        return;
    }

    int i;

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


static int sendPacket(InstaMsg *c, unsigned char *buf, int length)
{
    if((c->ipstack).socketCorrupted == 1)
    {
        return FAILURE;
    }

    int rc = SUCCESS;
    if((c->ipstack).write(&(c->ipstack), buf, length) == FAILURE)
    {
        (c->ipstack).socketCorrupted = 1;
        rc = FAILURE;
    }

    return rc;
}


static int readPacket(InstaMsg* c, MQTTFixedHeader *fixedHeader)
{
    if((c->ipstack).socketCorrupted == 1)
    {
        return FAILURE;
    }

    MQTTHeader header = {0};
    int rc = FAILURE;
    int len = 0;
    int rem_len = 0;

    /*
     * 0. Before reading the packet, memset the read-buffer to all-empty, else there will be issues
     *    processing the buffer as a string.
     */
    memset(c->readbuf, 0, MAX_BUFFER_SIZE);


    /*
     * 1. read the header byte.  This has the packet type in it.
     */
    int numRetries = MAX_TRIES_ALLOWED_WHILE_READING_FROM_NETWORK_MEDIUM;
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
    unsigned char i;
    int multiplier = 1;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    rem_len = 0;
    do
    {
        if((c->ipstack).read(&(c->ipstack), &i, 1, 1) == FAILURE) // Pseudo-Blocking Call
        {
            (c->ipstack).socketCorrupted = 1;
            goto exit;
        }

        rem_len += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);


    len += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if(rem_len > 0)
    {
        if((c->ipstack).read(&(c->ipstack), c->readbuf + len, rem_len, 1) == FAILURE) // Pseudo-Blocking Call
        {
            (c->ipstack).socketCorrupted = 1;
            goto exit;
        }
    }

    header.byte = c->readbuf[0];
    fillFixedHeaderFieldsFromPacketHeader(fixedHeader, &header);

    rc = SUCCESS;

exit:
    return rc;
}


// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
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
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}


static int deliverMessageToSelf(InstaMsg* c, MQTTString* topicName, MQTTMessage* message)
{
    int i;
    int rc = FAILURE;

    // we have to find the right message handler - indexed by topic
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
    enum QoS qos = (message->fixedHeaderPlusMsgId).fixedHeader.qos;
    if (qos != QOS0)
    {
        char buf[MAX_BUFFER_SIZE];
        int len;

        if (qos == QOS1)
        {
            len = MQTTSerialize_ack(buf, MAX_BUFFER_SIZE, PUBACK, 0, (message->fixedHeaderPlusMsgId).msgId);
        }
        else if (qos == QOS2)
        {
            len = MQTTSerialize_ack(buf, MAX_BUFFER_SIZE, PUBREC, 0, (message->fixedHeaderPlusMsgId).msgId);
        }

        if (len > 0)
        {
            rc = sendPacket(c, buf, len);
        }
    }

    return rc;
}


static int fireResultHandlerUsingMsgIdAsTheKey(InstaMsg *c)
{
    int msgId = -1;

    MQTTFixedHeaderPlusMsgId fixedHeaderPlusMsgId;
    if (MQTTDeserialize_FixedHeaderAndMsgId(&fixedHeaderPlusMsgId, c->readbuf, MAX_BUFFER_SIZE) == SUCCESS)
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

    char replyTopic[MAX_BUFFER_SIZE] = {0};
    char messageId[MAX_BUFFER_SIZE] = {0};
    char method[MAX_BUFFER_SIZE] = {0};
    char url[MAX_BUFFER_SIZE] = {0};
    char filename[MAX_BUFFER_SIZE] = {0};


    getJsonKeyValueIfPresent(msg->payload, REPLY_TOPIC, replyTopic);
    getJsonKeyValueIfPresent(msg->payload, MESSAGE_ID, messageId);
    getJsonKeyValueIfPresent(msg->payload, METHOD, method);
    getJsonKeyValueIfPresent(msg->payload, "url", url);
    getJsonKeyValueIfPresent(msg->payload, "filename", filename);

    if(strlen(replyTopic) == 0)
    {
        return logJsonFailureMessageAndReturn(REPLY_TOPIC, msg);
    }
    if(strlen(messageId) == 0)
    {
        return logJsonFailureMessageAndReturn(MESSAGE_ID, msg);
    }
    if(strlen(method) == 0)
    {
        return logJsonFailureMessageAndReturn(METHOD, msg);
    }


    unsigned char ackMessage[MAX_BUFFER_SIZE] = {0};

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
        HTTPResponse response = downloadFile(url, filename, NULL, NULL, 10);
        if(response.status == HTTP_FILE_DOWNLOAD_SUCCESS)
        {
            ackStatus = 1;
        }
        sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": %d}", messageId, ackStatus);

    }
    else if( (strcmp(method, "GET") == 0) && (strlen(filename) == 0))
    {
        unsigned char fileList[MAX_BUFFER_SIZE] = {0};
        (c->singletonUtilityFs).getFileListing(&(c->singletonUtilityFs), fileList, MAX_BUFFER_SIZE, ".");

        info_log(FILE_LISTING ": [%s]", fileList);

        sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": 1, \"files\": %s}", messageId, fileList);
    }
    else if( (strcmp(method, "DELETE") == 0) && (strlen(filename) > 0))
    {
        int status = (c->singletonUtilityFs).deleteFile(&(c->singletonUtilityFs), filename);
        if(status == SUCCESS)
        {
            info_log(FILE_DELETE "[%s] deleted successfully.", filename);
            sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": 1}", messageId);
        }
        else
        {
            error_log(FILE_DELETE "[%s] could not be deleted :(", filename);
            sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": 0, \"error_msg\":\"%s\"}", messageId, "File-Removal Failed :(");
        }
    }
    else if( (strcmp(method, "GET") == 0) && (strlen(filename) > 0))
    {
        char clientIdNotSplitted[MAX_BUFFER_SIZE] = {0};
        sprintf(clientIdNotSplitted, "%s-%s", c->connectOptions.clientID.cstring, c->connectOptions.username.cstring);

        KeyValuePairs headers[] = {
                                    {
                                        "Authorization",
                                        c->connectOptions.password.cstring
                                    },
                                    {
                                        "ClientId",
                                        clientIdNotSplitted
                                    },
                                    {
                                        "Content-Type",
                                        "multipart/form-data; boundary=" POST_BOUNDARY
                                    },
                                    {
                                        CONTENT_LENGTH,
                                        "0"                 // This will be updated to proper bytes later.
                                    },
                                    {
                                        0
                                    }
                                  };

        HTTPResponse response = uploadFile(c->fileUploadUrl, filename, NULL, headers, 10);
        if(response.status == HTTP_FILE_UPLOAD_SUCCESS)
        {
            sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": 1, \"url\": \"%s\"}", messageId, response.body);
        }
        else
        {
            sprintf(ackMessage, "{\"response_id\": \"%s\", \"status\": 0}", messageId);
        }
    }


    /*
     * Send the acknowledgement, along with the ackStatus (success/failure).
     */
    MQTTPublish(c,
                replyTopic,
                ackMessage,
                (msg->fixedHeaderPlusMsgId).fixedHeader.qos,
                (msg->fixedHeaderPlusMsgId).fixedHeader.dup,
                NULL,
                MQTT_RESULT_HANDLER_TIMEOUT,
                0,
                1);
}


void removeExpiredResultHandlers(InstaMsg *c)
{
    int i;
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->resultHandlers[i].msgId > 0)
        {
            info_log("No result obtained for msgId [%u] in the specified period", c->resultHandlers[i].msgId);
            c->resultHandlers[i].msgId = 0;
        }
    }
}


void sendPingReqToServer(InstaMsg *c)
{
    unsigned char buf[1000];
    int len = MQTTSerialize_pingreq(buf, 1000);
    if (len > 0)
    {
        sendPacket(c, buf, len);
    }
}


void clearInstaMsg(InstaMsg *c)
{
    release_network(&(c->ipstack));
    release_system_utils(&(c->systemUtils));
    release_file_system(&(c->singletonUtilityFs));
    release_timer(&(c->singletonUtilityTimer));

    if(serialLoggerEnabled == 1)
    {
        release_serial_logger(&serialLogger);
    }
    else
    {
        release_file_logger(&fileLogger);
    }
}


void initInstaMsg(InstaMsg* c,
                  char *clientId,
                  char *authKey,
                  int (*connectHandler)(),
                  int (*disconnectHandler)(),
                  int (*oneToOneMessageHandler)(),
                  struct opts_struct *opts)
{
    int i;

    // VERY IMPORTANT: If this is not done, the "write" on an invalid socket will cause program-crash
    signal(SIGPIPE,SIG_IGN);

    currentLogLevel = LOG_LEVEL;
    {
        serialLoggerEnabled = USE_SERIAL_LOGGER;
        if(serialLoggerEnabled == 1)
        {
            init_serial_logger(&serialLogger, opts->logFilePath);

            logger_write_func = (void *) &(serialLogger.serial.write);
            logger_medium = &(serialLogger.serial);
        }
        else
        {
            init_file_logger(&fileLogger, opts->logFilePath);

            logger_write_func = (void *) &(fileLogger.fs.write);
            logger_medium = &(fileLogger.fs);
        }
    }

    init_timer(&(c->singletonUtilityTimer), NULL);
    init_file_system(&(c->singletonUtilityFs), "");
    init_system_utils(&(c->systemUtils), NULL);
	init_network(&(c->ipstack), INSTAMSG_HOST, INSTAMSG_PORT);

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        c->messageHandlers[i].msgId = 0;
        c->messageHandlers[i].topicFilter = 0;

        c->resultHandlers[i].msgId = 0;
        c->resultHandlers[i].timeout = 0;
    }

    c->defaultMessageHandler = NULL;
    c->next_packetid = MAX_PACKET_ID;
    c->onConnectCallback = connectHandler;
    c->onDisconnectCallback = disconnectHandler;
    c->oneToOneMessageCallback = oneToOneMessageHandler;

    memset(c->filesTopic, 0, MAX_BUFFER_SIZE);
    sprintf(c->filesTopic, "instamsg/clients/%s/files", clientId);

    memset(c->rebootTopic, 0, MAX_BUFFER_SIZE);
    sprintf(c->rebootTopic, "instamsg/clients/%s/reboot", clientId);

    memset(c->enableServerLoggingTopic, 0, MAX_BUFFER_SIZE);
    sprintf(c->enableServerLoggingTopic, "instamsg/clients/%s/enableServerLogging", clientId);

    memset(c->serverLogsTopic, 0, MAX_BUFFER_SIZE);
    sprintf(c->serverLogsTopic, "instamsg/clients/%s/logs", clientId);

    memset(c->fileUploadUrl, 0, MAX_BUFFER_SIZE);
    sprintf(c->fileUploadUrl, "/api/beta/clients/%s/files", clientId);

    c->serverLoggingEnabled = 0;

	c->connectOptions.willFlag = 0;
	c->connectOptions.MQTTVersion = 3;
	c->connectOptions.cleansession = 1;

    memset(c->clientIdMachine, 0, MAX_BUFFER_SIZE);
    strncpy(c->clientIdMachine, clientId, 23);
    c->connectOptions.clientID.cstring = c->clientIdMachine;

    memset(c->username, 0, MAX_BUFFER_SIZE);
    strcpy(c->username, clientId + 24);
    c->connectOptions.username.cstring = c->username;

    memset(c->password, 0, MAX_BUFFER_SIZE);
    strcpy(c->password, authKey);
    c->connectOptions.password.cstring = c->password;

    MQTTConnect(c);
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
                if (MQTTDeserialize_connack((unsigned char*)&sessionPresent, &connack_rc, c->readbuf, MAX_BUFFER_SIZE) == 1)
                {
                    if(connack_rc == 0x00)  // Connection Accepted
                    {
                        if(c->onConnectCallback != NULL)
                        {
                            c->onConnectCallback();
                        }
                    }
                    else
                    {
                        info_log("Client-Connection failed with code [%d]", connack_rc);
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
                fireResultHandlerUsingMsgIdAsTheKey(c);

                /*
                * Remove the message-handlers, if the server was unable to process the subscription-request.
                */
                int count = 0, grantedQoS = -1;
                unsigned short msgId;

                if (MQTTDeserialize_suback(&msgId, 1, &count, &grantedQoS, c->readbuf, MAX_BUFFER_SIZE) != 1)
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
                MQTTString topicPlusPayload;
                MQTTMessage msg;
                if (MQTTDeserialize_publish(&(msg.fixedHeaderPlusMsgId),
                                            &topicPlusPayload,
                                            (unsigned char**)&msg.payload,
                                            (int*)&msg.payloadlen,
                                            c->readbuf,
                                            MAX_BUFFER_SIZE) != SUCCESS)
                {
                    goto exit;
                }

                /*
                 * At this point, "msg.payload" contains the real-stuff that is passed from the peer ....
                 */
                char topicName[MAX_BUFFER_SIZE] = {0};
                snprintf(topicName, strlen(topicPlusPayload.lenstring.data) - strlen(msg.payload) + 1, "%s", topicPlusPayload.lenstring.data);

                if(topicName != NULL)
                {
                    if(strcmp(topicName, c->filesTopic) == 0)
                    {
                        handleFileTransfer(c, &msg);
                        break;
                    }
                    else if(strcmp(topicName, c->enableServerLoggingTopic) == 0)
                    {
                        serverLoggingTopicMessageArrived(c, &msg);
                        break;
                    }
                    else if(strcmp(topicName, c->rebootTopic) == 0)
                    {
                        (c->systemUtils).rebootDevice(&(c->systemUtils));
                        break;
                    }
                }

                /*
                 * This is the last-ditch effort... if we reach till here... call this method
                 */
                deliverMessageToSelf(c, &topicPlusPayload, &msg);

                break;
            }

            case PUBREC:
            {
                int msgId = fireResultHandlerUsingMsgIdAsTheKey(c);

                char buf[MAX_BUFFER_SIZE];
                if ((len = MQTTSerialize_ack(buf, MAX_BUFFER_SIZE, PUBREL, 0, msgId)) <= 0)
                {
                    goto exit;
                }

                attachResultHandler(c, msgId, MQTT_RESULT_HANDLER_TIMEOUT, publishQoS2CycleCompleted);
                sendPacket(c, buf, len); // send the PUBREL packet

                break;
            }

            case PUBCOMP:
            {
                fireResultHandlerUsingMsgIdAsTheKey(c);
                break;
            }

            case PINGRESP:
            {
                info_log("PINGRESP received... relations are intact !!");
                break;
            }
        }
    } while(rc == SUCCESS); // Keep reading packets till the time we are receiving packets fine.

exit:
        return;
}


void* MQTTConnect(void* arg)
{
    char buf[MAX_BUFFER_SIZE];
    int len = 0;

    InstaMsg *c = (InstaMsg *)arg;
    if ((len = MQTTSerialize_connect(buf, MAX_BUFFER_SIZE, &(c->connectOptions))) <= 0)
    {
        return;
    }

    sendPacket(c, buf, len);

    return NULL;
}


int MQTTSubscribe(InstaMsg* c,
                  const char* topicName,
                  const enum QoS qos,
                  messageHandler messageHandler,
                  void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                  unsigned int resultHandlerTimeout)
{
    int rc = FAILURE;
    int len = 0;
    int id;
    char buf[MAX_BUFFER_SIZE];

    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;

    id = getNextPacketId(c);
    len = MQTTSerialize_subscribe(buf, MAX_BUFFER_SIZE, 0, id, 1, &topic, (int*)&qos);
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

    if ((rc = sendPacket(c, buf, len)) != SUCCESS) // send the subscribe packet
        goto exit;             // there was a problem

exit:
    return rc;
}


int MQTTUnsubscribe(InstaMsg* c, const char* topicFilter)
{
    int rc = FAILURE;
    char buf[MAX_BUFFER_SIZE];

    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;

    if ((len = MQTTSerialize_unsubscribe(buf, MAX_BUFFER_SIZE, 0, getNextPacketId(c), 1, &topic)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, buf, len)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem

exit:
    return rc;
}


int MQTTPublish(InstaMsg* c,
                const char* topicName,
                const char* payload,
                const enum QoS qos,
                const char dup,
                void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                unsigned int resultHandlerTimeout,
                const char retain,
                const char logging)
{
    int rc = FAILURE;
    char buf[MAX_BUFFER_SIZE];

    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;
    int id;

    if (qos == QOS1 || qos == QOS2)
    {
        id = getNextPacketId(c);

        /*
         * We will get PUBACK from server only for QOS1 and QOS2.
         * So, it makes sense to lodge the result-handler only for these cases.
         */
        attachResultHandler(c, id, resultHandlerTimeout, resultHandler);
    }

    len = MQTTSerialize_publish(buf, MAX_BUFFER_SIZE, 0, qos, retain, id, topic, (unsigned char*)payload, strlen(payload) + 1);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, buf, len)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem

    if (qos == QOS1)
    {
    }
    else if (qos == QOS2)
    {
    }

exit:
    return rc;
}


int MQTTDisconnect(InstaMsg* c)
{
    int rc = FAILURE;
    char buf[MAX_BUFFER_SIZE];

    int len = MQTTSerialize_disconnect(buf, MAX_BUFFER_SIZE);
    if (len > 0)
        rc = sendPacket(c, buf, len);            // send the disconnect packet

    if(c->onDisconnectCallback != NULL)
    {
        c->onDisconnectCallback();
    }

    return rc;
}

