/*******************************************************************************
 * Copyright (c) 2012, 2013 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *    Ian Craggs - change delimiter option from char to string
 *    Al Stockdill-Mander - Version using the embedded C client
 *******************************************************************************/

/*

 stdout subscriber

 compulsory parameters:

  topic to subscribe to

 defaulted parameters:

	--host localhost
	--port 1883
	--qos 2
	--delimiter \n
	--clientid stdout_subscriber

	--userid none
	--password none

 for example:

    stdoutsub topic/of/interest --host iot.eclipse.org

*/
#include "src/common/include/instamsg.h"
#include "src/common/include/log.h"
#include "src/common/include/globals.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include <sys/time.h>




static void publishAckReceived(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    info_log("PUBACK received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
}


void* coreLoopyBusinessLogicInitiatedBySelf(void *arg)
{
}


static void subscribeAckReceived(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    info_log("SUBACK received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
}


static void messageArrived(MessageData* md)
{
	MQTTMessage* message = md->message;
    info_log("%.*s", (int)message->payloadlen, (char*)message->payload);
}


static int onConnectOneTimeOperations()
{
    info_log("Connected successfully to InstaMsg-Server.");
}


static int onDisconnect()
{
    info_log("Disconnect \"callback\" called.. not really needed, as MQTT-Disconnect does not get any response from server. "
           "So, no async-callback required as such.\n");

	return 0;
}


int main(int argc, char** argv)
{
    SYSTEM_GLOBAL_INIT();

    while(1)
    {
        initInstaMsg(&instaMsg, "00125580-e29a-11e4-ace1-bc764e102b63", "password", onConnectOneTimeOperations, onDisconnect, NULL, NULL);

        while(1)
        {
            /*
            if(instaMsg.ipstack.socketCorrupted == 1)
            {
                clearInstaMsg(&instaMsg);
                break;
            }

            // InstaMsg-Specific Cycles
            readAndProcessIncomingMQTTPacketsIfAny(&instaMsg);
            removeExpiredResultHandlers(&instaMsg);

            if(instaMsg.connected == 1)
            {
                sendPingReqToServer(&instaMsg);

                // Application-Specific Cycles
                coreLoopyBusinessLogicInitiatedBySelf(NULL);
            }
            else
            {
                static int connectionAttempts = 0;
                connectionAttempts++;

                error_log("Network is fine at physical layer, but no connection established (yet) with InstaMsg-Server.");
                if(connectionAttempts > MAX_CONN_ATTEMPTS_WITH_PHYSICAL_LAYER_FINE)
                {
                    connectionAttempts = 0;
                    error_log("Connection-Attempts exhausted ... so trying with re-initializing the network-physical layer.");

                    instaMsg.ipstack.socketCorrupted = 1;
                }
            }
            */

            info_log("Yeahhh... we could proceed to business-logic now !!");
        }
    }
}

