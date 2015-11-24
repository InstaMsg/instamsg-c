/*******************************************************************************
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
    } oneToOneResponseHandlers[MAX_MESSAGE_HANDLERS];

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

    unsigned char serverLoggingEnabled;

    Socket ipstack;
#ifdef FILE_SYSTEM_INTERFACE_ENABLED
    FileSystem singletonUtilityFs;
#endif

    MQTTPacket_connectData connectOptions;

    char clientIdComplete[MAX_CLIENT_ID_SIZE];
    char clientIdMachine[MAX_CLIENT_ID_SIZE];
    char username[MAX_CLIENT_ID_SIZE];

    char password[MAX_CLIENT_ID_SIZE];

    unsigned char connected;
};

InstaMsg instaMsg;
void* MQTTConnect(void* arg);



void clearInstaMsg(InstaMsg *c);
void initInstaMsg(InstaMsg* c,
                  int (*connectHandler)(),
                  int (*disconnectHandler)(),
                  int (*oneToOneMessageHandler)());


#define SERVER_LOGGING  "[SERVER-LOGGING] "
#define FILE_TRANSFER   "[FILE-TRANSFER] "
#define FILE_DOWNLOAD   "[FILE-DOWNLOAD] "
#define FILE_UPLOAD     "[FILE-UPLOAD] "
#define FILE_LISTING    "[FILE-LISTING] "
#define FILE_DELETE     "[FILE-DELETE] "

void readAndProcessIncomingMQTTPacketsIfAny(InstaMsg* c);
void sendPingReqToServer(InstaMsg *c);
void start(int (*onConnectOneTimeOperations)(),
           int (*onDisconnect)(),
           int (*oneToOneMessageHandler)(OneToOneResult *),
           void (*coreLoopyBusinessLogicInitiatedBySelf)(),
           int businessLogicInterval);

int MQTTDisconnect ();







/*
 *********************************************************************************************************************
 **************************************** PUBLIC APIs *****************************************************************
 **********************************************************************************************************************
 */


/*
 *
 * topicName                            :
 *
 *      Topic on which the message should be published
 *      FOR SUCCESSFUL PUBLISHING TO A TOPIC, THE TOPIC MUST BE IN THE "Pub Topics" LIST ON INSTAMSG-SERVER.
 *
 *
 * payload                              :
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
 * retain                               :
 *
 *      Either 0 or 1.
 *      Meaning of this variable as per the spec at http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html
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
int MQTTPublish     (const char *topicName,
                     const char *payload,
                     const enum QoS qos,
                     const char dup,
                     void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                     unsigned int resultHandlerTimeout,
                     const char retain,
                     const char logging);






/*
 *
 * peer                                 :
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
 * payload                              :
 *
 *      Message-Content
 *
 *
 * oneToOneResponseHandler              :
 *
 *      Callback function-pointer.
 *      Called when the remote-peer has sent back a message to the local-peer.
 *
 *
 * oneToOneResponseHandlerTimeout       :
 *
 *      Time for which "oneToOneResponseHandler" remains active.
 *
 *      If remote-peer does not respond within this interval, "oneToOneResponseHandler" will be removed, and will never be called
 *      even if remote-peer sends something at some later stage > "oneToOneResponseHandlerTimeout" seconds).
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
int MQTTSend        (const char* peer,
                     const char* payload,
                     void (*oneToOneResponseHandler)(OneToOneResult *),
                     unsigned int oneToOneResponseHandlerTimeout);






/*
 *
 * topicName                            :
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
int MQTTSubscribe   (const char *topicName,
                     const enum QoS qos,
                     void (*messageHandler)(MessageData *),
                     void (*resultHandler)(MQTTFixedHeaderPlusMsgId *),
                     unsigned int resultHandlerTimeout,
                     const char logging);






/*
 * topicName                            :
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

 */
int MQTTUnsubscribe (const char *topicName);






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
    char *peer;
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
    void (*reply)(OneToOneResult *oneToOneResult, const char *replyMessage);
};

#endif
