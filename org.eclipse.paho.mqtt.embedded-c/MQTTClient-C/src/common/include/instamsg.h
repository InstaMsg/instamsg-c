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
 *******************************************************************************/

#ifndef __MQTT_CLIENT_C_
#define __MQTT_CLIENT_C_

#include "../../../../MQTTPacket/src/MQTTPacket.h"
#include "../../time/include/time.h"
#include "../../network/include/network.h"
#include "../../threading/include/threading.h"

#include "stdio.h"

#define MAX_PACKET_ID 30000
#define MAX_MESSAGE_HANDLERS 5
#define MAX_BUFFER_SIZE 100
#define KEEP_ALIVE_INTERVAL_SECS 10
#define INSTAMSG_RESULT_HANDLER_TIMEOUT_SECS 10


void NewTimer(Timer*);

typedef struct MQTTMessage MQTTMessage;

typedef struct MessageData MessageData;

struct MQTTMessage
{
    MQTTFixedHeaderPlusMsgId fixedHeaderPlusMsgId;
    void *payload;
    size_t payloadlen;
};

struct MessageData
{
    MQTTMessage* message;
    MQTTString* topicName;
};

typedef void (*messageHandler)(MessageData*);

typedef struct InstaMsg InstaMsg;
struct InstaMsg {
    unsigned int next_packetid;
    unsigned char readbuf[MAX_BUFFER_SIZE];
    int isconnected;

    struct MessageHandlers
    {
        int msgId;
        const char* topicFilter;
        void (*fp) (MessageData*);
    } messageHandlers[MAX_MESSAGE_HANDLERS];      // Message handlers are indexed by subscription topic

    struct ResultHandlers
    {
        int msgId;
        unsigned int timeout;
        void (*fp) (MQTTFixedHeaderPlusMsgId*);
    } resultHandlers[MAX_MESSAGE_HANDLERS];      // Message handlers are indexed by subscription topic

    void (*defaultMessageHandler) (MessageData*);
    int (*onConnectCallback)();
    int (*onDisconnectCallback)();
    int (*oneToOneMessageCallback)();

    struct Mutex *sendPacketMutex;
    struct Mutex *messageHandlersMutex;
    struct Mutex *resultHandlersMutex;

    Network* ipstack;
};


int MQTTConnect (InstaMsg*, MQTTPacket_connectData*);

int MQTTPublish(InstaMsg *c,
                const char *topicName,
                const char *payload,
                const enum QoS qos,
                const char dup,
                void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                unsigned int resultHandlerTimeout,
                const char retain,
                const char logging);


int MQTTSubscribe(InstaMsg *c,
                  const char *topicName,
                  const enum QoS qos,
                  messageHandler messageHandler,
                  void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                  unsigned int resultHandlerTimeout);

int MQTTUnsubscribe (InstaMsg*, const char*);
int MQTTDisconnect (InstaMsg*);
void setDefaultMessageHandler(InstaMsg*, messageHandler);

void initInstaMsg(InstaMsg* c,
                  Network* network,
                  char *clientId,
                  char *authKey,
                  int (*connectHandler)(),
                  int (*disconnectHandler)(),
                  int (*oneToOneMessageHandler)());

void* clientTimerThread(InstaMsg *c);
void* keepAliveThread(InstaMsg *c);
void readPacketThread(InstaMsg *c);

#define DefaultClient {0, 0, 0, 0, NULL, NULL, 0, 0, 0}
#endif
