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
typedef struct OneToOneResult OneToOneResult;
typedef struct InstaMsg InstaMsg;


struct MQTTMessage
{
    MQTTFixedHeaderPlusMsgId fixedHeaderPlusMsgId;
    void *payload;
    size_t payloadlen;
};


struct InstaMsg {
    unsigned int next_packetid;
#if SSL_ENABLED == 1
    unsigned char readbuf[4000];
#else
    unsigned char readbuf[MAX_BUFFER_SIZE];
#endif

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
        int (*fp) (OneToOneResult*);
    } oneToOneHandlers[MAX_MESSAGE_HANDLERS];

    void (*defaultMessageHandler) (MessageData*);
    int (*onConnectCallback)();
    int (*onDisconnectCallback)();
    int (*oneToOneMessageHandler)();

    char filesTopic[100];
    char rebootTopic[100];
    char enableServerLoggingTopic[100];
    char serverLogsTopic[100];
    char fileUploadUrl[100];
    char receiveConfigTopic[100];
#if MEDIA_STREAMING_ENABLED == 1
    char mediaTopic[100];
    char mediaReplyTopic[100];
    char mediaStopTopic[100];
    char mediaPauseTopic[100];
    char mediaStreamsTopic[100];

    char selfIpAddress[20];
    char mediaServerIpAddress[20];
    char mediaServerPort[10];
#endif

    unsigned char serverLoggingEnabled;

    Socket timeSyncerSocket;
    Socket ipstack;
#if FILE_SYSTEM_ENABLED == 1
    FileSystem singletonUtilityFs;
#endif

    MQTTPacket_connectData connectOptions;

    char clientIdComplete[MAX_CLIENT_ID_SIZE];
    char clientIdMachine[MAX_CLIENT_ID_SIZE];
    char username[MAX_CLIENT_ID_SIZE];

    char password[MAX_CLIENT_ID_SIZE];

    unsigned char connected;

    long FRESH_SERVER_LOGS_TIME;
    long serverLogsStartTime;
};

extern InstaMsg instaMsg;
extern volatile unsigned char runBusinessLogicImmediately;
void* MQTTConnect(void* arg);



void clearInstaMsg(InstaMsg *c);
void initInstaMsg(InstaMsg* c,
                  int (*connectHandler)(),
                  int (*disconnectHandler)(),
                  int (*oneToOneHandler)());


#define SERVER_LOGGING  PROSTR("[SERVER-LOGGING] ")
#define FILE_TRANSFER   PROSTR("[FILE-TRANSFER] ")
#define FILE_DOWNLOAD   PROSTR("[FILE-DOWNLOAD] ")
#define FILE_UPLOAD     PROSTR("[FILE-UPLOAD] ")
#define FILE_LISTING    PROSTR("[FILE-LISTING] ")
#define FILE_DELETE     PROSTR("[FILE-DELETE] ")

void readAndProcessIncomingMQTTPacketsIfAny(InstaMsg* c);
void sendPingReqToServer(InstaMsg *c);

int disconnect ();







/*
 *********************************************************************************************************************
 **************************************** PUBLIC APIs *****************************************************************
 **********************************************************************************************************************
 */


/*
 *
 * topic                                :
 *
 *      Topic on which the message should be published
 *      FOR SUCCESSFUL PUBLISHING TO A TOPIC, THE TOPIC MUST BE IN THE "Pub Topics" LIST ON INSTAMSG-SERVER.
 *
 *
 * msg                                  :
 *
 *      Message-Content
 *
 *
 * qos                                  :
 *
 *      One of QOS0, QOS1, QOS2.
 *      Meanings of these levels as per the spec at http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html
 *
 *
 * dup                                  :
 *
 *      Either 0 or 1.
 *      Meaning of this variable as per the spec at http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html
 *
 *
 * resultHandler                        :
 *
 *      Callback function-pointer.
 *      Called when the message is published to the server, and the server responds with a PUBACK.
 *
 *      No effect for QOS0.
 *
 *
 * resultHandlerTimeout                 :
 *
 *      Time for which "resultHandler" remains active.
 *
 *      If PUBACK is not received within this interval, "resultHandler" will be removed, and will never be called for the
 *      message under consideration (even if PUBACK is received at some later stage > "resultHandlerTimeout" seconds).
 *
 *
 * logging                              :
 *
 *      Either 0 or 1.
 *      Specified whether logging should be done in the client-logger, about the progress of message bein published (or not !!!)
 *
 *      Highly recommended to have this value as 1, for easy tracking/debugging.
 *
 *
 *
 * Returns:
 *
 *      SUCCESS   :: If the publish-packet is successfully encoded AND sent over the wire to the InstaMsg-Server.
 *      FAILURE   :: If any of the above steps fails.
 *
 *
 *      Note that in case of FAILURE,
 *
 *      a)
 *      The application MUST ""NOT"" do any socket-reinitialization (or anything related).
 *      It will be handled autimatically by the InstaMsg-Driver code (as long as the device-implementor has implemented all
 *      the InstaMsg-Kernel APIs in accordance with the requirements).
 *
 *      b)
 *      It is the application's duty to re-send the message (if at all required), because there is no guarantee whether
 *      the message reached the server or not.
 *
 *
 * Kindly see
 *
 *                  common/apps/publisher/main.c
 *
 *      for simple (yet complete) example-usage.
 *
 */
int publish         (const char *topic,
                     const char *msg,
                     const int qos,
                     const char dup,
                     void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                     unsigned int resultHandlerTimeout,
                     const char logging);






/*
 * A useful-utility function using the "publish" API, that should suffice for most business-applications.
 */
int publishMessageWithDeliveryGuarantee(char *topic, char *payload);






/*
 *
 * peerClientId                         :
 *
 *      Peer-Id.
 *
 *      This value is equal to the client-id of the peer.
 *      The client-id is generated by the InstaMsg-Server, and so the local-peer must have the exact client-id value of
 *      the remote-peer.
 *
 *      FOR SUCCESSFUL SENDING TO THE PEER, THE PEER CLIENT-ID MUST BE LISTED AS ONE OF THE "Pub Topics" ON INSTAMSG-SERVER.
 *
 *
 * msg                                  :
 *
 *      Message-Content
 *
 *
 * replyHandler                         :
 *
 *      Callback function-pointer.
 *      Called when the remote-peer has sent back a message to the local-peer.
 *
 *
 * replyHandlerTimeout                  :
 *
 *      Time for which "oneToOneHandler" remains active.
 *
 *      If remote-peer does not respond within this interval, "oneToOneHandler" will be removed, and will never be called
 *      even if remote-peer sends something at some later stage > "oneToOneHandlerTimeout" seconds).
 *
 *
 *
 * Returns:
 *
 *      SUCCESS   :: If the send-packet is successfully encoded AND sent over the wire to the InstaMsg-Server.
 *      FAILURE   :: If any of the above steps fails.
 *
 *
 *      Note that in case of FAILURE,
 *
 *      a)
 *      The application MUST ""NOT"" do any socket-reinitialization (or anything related).
 *      It will be handled autimatically by the InstaMsg-Driver code (as long as the device-implementor has implemented all
 *      the InstaMsg-Kernel APIs in accordance with the requirements).
 *
 *      b)
 *      It is the application's duty to re-send the message (if at all required), because there is no guarantee whether
 *      the message reached the server or not.
 *
 *
 * Kindly see
 *
 *                  common/apps/one_to_one_initiator/main.c
 *                  common/apps/subscriber/main.c
 *
 *      for simple (yet complete) example-usage.
 *
 */
int sendOneToOne     (const char* peerClientId,
                     const char* msg,
                     int (*replyHandler)(OneToOneResult *),
                     unsigned int replyHandlerTimeout);






/*
 *
 * topic                                :
 *
 *      Topic on which client needs to subscribe.
 *      FOR SUCCESSFUL SUBSCRIBING TO A TOPIC, THE TOPIC MUST BE IN THE "Sub Topics" LIST ON INSTAMSG-SERVER.
 *
 *
 * qos                                  :
 *
 *      One of QOS0, QOS1, QOS2.
 *      Meanings of these levels as per the spec at http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html
 *
 *
 * messageHandler                       :
 *
 *      Callback function-pointer.
 *      Called whenever a message arrives on the subscribed-topic from InstaMsg-Server.
 *
 *
 * resultHandler                        :
 *
 *      Callback function-pointer.
 *      Called when the message is published to the server, and the server responds with a SUBACK.
 *
 *      No effect for QOS0.
 *
 *
 * resultHandlerTimeout                 :
 *
 *      Time for which "resultHandler" remains active.
 *
 *      If SUBACK is not received within this interval, "resultHandler" will be removed, and will never be called for the
 *      message under consideration (even if PUBACK is received at some later stage > "resultHandlerTimeout" seconds).
 *
 *
 * logging                              :
 *
 *      Either 0 or 1.
 *      Specified whether logging should be done in the client-logger, about the progress of message bein published (or not !!!)
 *
 *      Highly recommended to have this value as 1, for easy tracking/debugging.
 *
 *
 *
 * Returns:
 *
 *      SUCCESS   :: If the subscription-packet is successfully encoded AND sent over the wire to the InstaMsg-Server.
 *      FAILURE   :: If any of the above steps fails.
 *
 *
 *      Note that in case of FAILURE,
 *
 *      a)
 *      The application MUST ""NOT"" do any socket-reinitialization (or anything related).
 *      It will be handled autimatically by the InstaMsg-Driver code (as long as the device-implementor has implemented all
 *      the InstaMsg-Kernel APIs in accordance with the requirements).
 *
 *      b)
 *      It is the application's duty to re-send the message (if at all required), because there is no guarantee whether
 *      the message reached the server or not.

 *
 *
 * Kindly see
 *
 *                  common/apps/subscriber/main.c
 *
 *      for simple (yet complete) example-usage.
 *
 */
int subscribe       (const char *topic,
                     const int qos,
                     void (*messageHandler)(MessageData *),
                     void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                     unsigned int resultHandlerTimeout,
                     const char logging);






/*
 * topic                                :
 *
 *      The topic on which the client needs to unsubscribe.
 *
 *
 * Returns:
 *
 *      SUCCESS   :: If the unsubscribe-packet is successfully encoded AND sent over the wire to the InstaMsg-Server.
 *      FAILURE   :: If any of the above steps fails.
 *
 *
 *      Note that in case of FAILURE,
 *
 *      a)
 *      The application MUST ""NOT"" do any socket-reinitialization (or anything related).
 *      It will be handled autimatically by the InstaMsg-Driver code (as long as the device-implementor has implemented all
 *      the InstaMsg-Kernel APIs in accordance with the requirements).
 *
 *      b)
 *      It is the application's duty to re-send the message (if at all required), because there is no guarantee whether
 *      the message reached the server or not.
 *
 */
int unsubscribe (const char *topic);






/*
 * Kindly see
 *
 *                  common/apps/subscriber/main.c
 *
 *      for simple (yet complete) example-usage.
 *
 */
struct MessageData
{
    MQTTMessage* message;
    MQTTString* topicName;
};






/*
 */
struct OneToOneResult
{
    /*
     ************** NOT EXPECTED TO BE USED BY THE APPLICATION ******************************
     */
    char *peerClientId;
    int peerMsgId;


    /*
     ************** EXPECTED TO BE USED BY THE APPLICATION **********************************
     */

    /*
     * Is one of 0, 1.
     *
     * 0 denotes that there was some error while fetching the response from peer.
     * 1 denotes that the response was succesfully received.
     */
    unsigned char succeeded;

    /*
     * Peer-Message.
     *
     * Makes sense only if the value of "succeeded" is 1.
     */
    char *peerMsg;

    /*
     * Function-Pointer, to send a reply to the peer.
     *
     * Kindly see
     *
     *                  common/apps/one_to_one_initiator/main.c
     *                  common/apps/subscriber/main.c
     *
     * for simple (yet complete) example-usage.
     */
    void (*reply)(OneToOneResult *oneToOneResult,
                  const char *msg,
                  int (*replyHandler)(OneToOneResult *),
                  unsigned int replyHandlerTimeout);
};






/*
 * Starting-Method For Application
 */
void start(int (*onConnectOneTimeOperations)(),
           int (*onDisconnect)(),
           int (*oneToOneHandler)(OneToOneResult *),
           void (*coreLoopyBusinessLogicInitiatedBySelf)(),
           int businessLogicInterval);

#endif
