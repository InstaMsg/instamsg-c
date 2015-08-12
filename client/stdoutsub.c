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


volatile int toStop = 0;

char *topic;

struct opts_struct *opts_p;

static void publishAckReceived(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    debug_log("PUBACK received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
}


void* coreLoopyBusinessLogicInitiatedBySelf(void *arg)
{
    int rc;

	if(opts_p->publish == 1)
	{
        char buf[MAX_BUFFER_SIZE] = {0};

        static int counter;
        counter++;
        sprintf(buf, "%s %d", opts_p->msg, counter);

		debug_log("Publishing message [%s] to %s", buf, topic);
		rc = MQTTPublish(&instaMsg, topic, (const char*)buf, opts_p->qos, 0,
                         publishAckReceived, MQTT_RESULT_HANDLER_TIMEOUT, 0, 1);
		debug_log("Published %d", rc);
	}
}


static int onConnect()
{
    info_log("Connected successfully");

	if(opts_p->subscribe == 1)
	{
    	info_log("Subscribing to %s", topic);
		int rc = MQTTSubscribe(&instaMsg, topic, opts_p->qos, messageArrived, subscribeAckReceived, MQTT_RESULT_HANDLER_TIMEOUT);
		info_log("Subscribed %d", rc);
	}
}


static int onDisconnect()
{
    info_log("Disconnect \"callback\" called.. not really needed, as MQTT-Disconnect does not get any response from server. "
           "So, no async-callback required as such.\n");

	return 0;
}


void usage()
{
	printf("MQTT stdout subscriber\n");
	printf("Usage: stdoutsub topicname <options>, where options are:\n");
	printf("  --host <hostname> (default is localhost)\n");
	printf("  --port <port> (default is 1883)\n");
	printf("  --qos <qos> (default is 2)\n");
	printf("  --delimiter <delim> (default is \\n)\n");
	printf("  --clientid <clientid> (default is hostname+timestamp)\n");
	printf("  --password none\n");
	printf("  --showtopics <on or off> (default is on if the topic has a wildcard, else off)\n");
	exit(-1);
}


void cfinish(int sig)
{
	signal(SIGINT, NULL);
	toStop = 1;
}


void getopts(int argc, char** argv, struct opts_struct *opts)
{
	int count = 2;

	while (count < argc)
	{
		if (strcmp(argv[count], "--qos") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "0") == 0)
					opts->qos = QOS0;
				else if (strcmp(argv[count], "1") == 0)
					opts->qos = QOS1;
				else if (strcmp(argv[count], "2") == 0)
					opts->qos = QOS2;
				else
					usage();
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--host") == 0)
		{
			if (++count < argc)
				opts->host = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--port") == 0)
		{
			if (++count < argc)
				opts->port = atoi(argv[count]);
			else
				usage();
		}
		else if (strcmp(argv[count], "--clientid") == 0)
		{
			if (++count < argc)
				opts->clientid = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--password") == 0)
		{
			if (++count < argc)
				opts->password = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--delimiter") == 0)
		{
			if (++count < argc)
				opts->delimiter = argv[count];
			else
				opts->nodelimiter = 1;
		}
		else if (strcmp(argv[count], "--showtopics") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "on") == 0)
					opts->showtopics = 1;
				else if (strcmp(argv[count], "off") == 0)
					opts->showtopics = 0;
				else
					usage();
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--sub") == 0)
		{
			opts->subscribe = 1;
		}
		else if (strcmp(argv[count], "--pub") == 0)
		{
			opts->publish = 1;
		}
        else if (strcmp(argv[count], "--msg") == 0)
        {
            if (++count < argc)
                strcpy(opts->msg, argv[count]);
            else
                usage();
        }
    	else if (strcmp(argv[count], "--log") == 0)
		{
			if (++count < argc)
				opts->logFilePath = argv[count];
			else
				usage();
		}

		count++;
	}
}


int main(int argc, char** argv)
{
    struct opts_struct opts =
    {
        (char*)"stdout-subscriber", 0, (char*)"\n", QOS2, NULL, (char*)"localhost", 1883, 0, 0, 0, ""
    };

	int rc = 0;
    char firstTimeStart = 1;

	if (argc < 2)
		usage();

	topic = argv[1];

	if (strchr(topic, '#') || strchr(topic, '+'))
		opts.showtopics = 1;
	if (opts.showtopics)
		printf("topic is %s\n", topic);

	getopts(argc, argv, &opts);
    opts_p = &opts;

	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);

	initInstaMsg(&instaMsg, opts.clientid, opts.password, onConnect, onDisconnect, NULL, &opts);

    while(1)
    {
        /*
         * InstaMsg-Specific Cycles
         */
        readAndProcessIncomingMQTTPacketsIfAny(&instaMsg);
        removeExpiredResultHandlers(&instaMsg);
        sendPingReqToServer(&instaMsg);

        /*
         * Application-Specific Cycles
         */
        coreLoopyBusinessLogicInitiatedBySelf(NULL);
        startAndCountdownTimer(3);
    }
}

