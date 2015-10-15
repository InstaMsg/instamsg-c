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

#ifndef INSTAMSG
#define INSTAMSG

#include "./globals.h"
#include "./log.h"
#include "../mqtt/src/MQTTPacket.h"

#include "./socket.h"
#include "./time.h"

#define MAX_PACKET_ID 10000
#define MAX_MESSAGE_HANDLERS 5
#define KEEP_ALIVE_INTERVAL_SECS 10
#define MQTT_RESULT_HANDLER_TIMEOUT 10



typedef struct MQTTMessage MQTTMessage;
typedef struct MessageData MessageData;
typedef struct InstaMsg InstaMsg;


struct MQTTMessage
{
    MQTTFixedHeaderPlusMsgId fixedHeaderPlusMsgId;
    void *payload;
    size_t payloadlen;
};


struct MessageData
{
    InstaMsg *c;
    MQTTMessage* message;
    MQTTString* topicName;
};


typedef void (*messageHandler)(MessageData*);

struct opts_struct
{
	int nodelimiter;
	char* delimiter;
	enum QoS qos;
	char* host;
	int port;
	int showtopics;

	int subscribe;
	int publish;
	char msg[100];
    char *logFilePath;
};

typedef struct OneToOneResult OneToOneResult;
struct OneToOneResult
{
    unsigned char succeeded;
    char *peerMsg;
    char *peer;
    int peerMsgId;
    void (*reply)(OneToOneResult *oneToOneResult, const char *replyMessage);
};

struct InstaMsg {
    unsigned int next_packetid;
    unsigned char readbuf[MAX_BUFFER_SIZE];

    struct MessageHandlers
    {
        int msgId;
        const char* topicFilter;
        void (*fp) (MessageData*);
    } messageHandlers[MAX_MESSAGE_HANDLERS];      /* Message handlers are indexed by subscription topic */

    struct ResultHandlers
    {
        int msgId;
        unsigned int timeout;
        void (*fp) (MQTTFixedHeaderPlusMsgId*);
    } resultHandlers[MAX_MESSAGE_HANDLERS];

    struct OneToOneHandlers
    {
        int msgId;
        unsigned int timeout;
        void (*fp) (OneToOneResult*);
    } oneToOneHandlers[MAX_MESSAGE_HANDLERS];

    void (*defaultMessageHandler) (MessageData*);
    int (*onConnectCallback)();
    int (*onDisconnectCallback)();
    int (*oneToOneMessageCallback)();

    char filesTopic[100];
    char rebootTopic[100];
    char enableServerLoggingTopic[100];
    char serverLogsTopic[100];
    char fileUploadUrl[100];

    unsigned char serverLoggingEnabled;

    Socket ipstack;
#ifdef FILE_SYSTEM_INTERFACE_ENABLED
    FileSystem singletonUtilityFs;
#endif

    MQTTPacket_connectData connectOptions;

    char clientIdComplete[MAX_CLIENT_ID_SIZE];
    char clientIdMachine[MAX_CLIENT_ID_SIZE];
    char username[MAX_CLIENT_ID_SIZE];

    char password[MAX_BUFFER_SIZE];

    unsigned char connected;
};

InstaMsg instaMsg;

void* MQTTConnect(void* arg);

int MQTTPublish(const char *topicName,
                const char *payload,
                const enum QoS qos,
                const char dup,
                void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                unsigned int resultHandlerTimeout,
                const char retain,
                const char logging);


int MQTTSubscribe(const char *topicName,
                  const enum QoS qos,
                  messageHandler messageHandler,
                  void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                  unsigned int resultHandlerTimeout,
                  const char logging);

int MQTTUnsubscribe (const char*);
int MQTTDisconnect ();
void setDefaultMessageHandler(InstaMsg*, messageHandler);

void clearInstaMsg(InstaMsg *c);
void initInstaMsg(InstaMsg* c,
                  int (*connectHandler)(),
                  int (*disconnectHandler)(),
                  int (*oneToOneMessageHandler)());


typedef struct JSONParseStuff JSONParseStuff;
struct JSONParseStuff
{
    const char *key;
    void *value;
    unsigned int mandatory;
    unsigned char type;
};


#define SERVER_LOGGING  "[SERVER-LOGGING] "
#define FILE_TRANSFER   "[FILE-TRANSFER] "
#define FILE_DOWNLOAD   "[FILE-DOWNLOAD] "
#define FILE_UPLOAD     "[FILE-UPLOAD] "
#define FILE_LISTING    "[FILE-LISTING] "
#define FILE_DELETE     "[FILE-DELETE] "

void readAndProcessIncomingMQTTPacketsIfAny(InstaMsg* c);
void removeExpiredResultHandlers(InstaMsg *c);
void sendPingReqToServer(InstaMsg *c);
void start(int (*onConnectOneTimeOperations)(),
           int (*onDisconnect)(),
           int (*oneToOneMessageHandler)(),
           void (*coreLoopyBusinessLogicInitiatedBySelf)(),
           int businessLogicInterval);

#define DefaultClient {0, 0, 0, 0, NULL, NULL, 0, 0, 0}
#endif
