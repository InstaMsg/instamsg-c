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
#include "../../specific/include/clienttime.h"
#include "../../specific/include/clientnetwork.h"

#include "stdio.h"

#define MAX_PACKET_ID 30000
#define MAX_MESSAGE_HANDLERS 5


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

typedef struct Client Client;

int MQTTConnect (Client*, MQTTPacket_connectData*);

int MQTTPublish(Client* c,
                const char* topicName,
                const char* payload,
                const enum QoS qos,
                const char dup,
                void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                unsigned int resultHandlerTimeout,
                const char retain,
                const char logging);


int MQTTSubscribe(Client* c,
                  const char* topicName,
                  const enum QoS qos,
                  messageHandler messageHandler,
                  void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                  unsigned int resultHandlerTimeout);

int MQTTUnsubscribe (Client*, const char*);
int MQTTDisconnect (Client*);
int MQTTYield (Client*, int);

void setDefaultMessageHandler(Client*, messageHandler);

void MQTTClient(Client*, Network*, unsigned int, unsigned char*, size_t, unsigned char*, size_t, int (*onConnect)());

struct Client {
    unsigned int next_packetid;
    unsigned int command_timeout_ms;
    size_t buf_size, readbuf_size;
    unsigned char *buf;
    unsigned char *readbuf;
    unsigned int keepAliveInterval;
    char ping_outstanding;
    int isconnected;

    struct MessageHandlers
    {
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

    Network* ipstack;
    Timer* ping_timer;
};

void* clientTimerThread(Client *c);
void cycle(Client *c);

#define DefaultClient {0, 0, 0, 0, NULL, NULL, 0, 0, 0}

#endif
