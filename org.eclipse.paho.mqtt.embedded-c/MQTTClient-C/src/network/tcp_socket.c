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
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#ifndef __MQTT_LINUX_
#define __MQTT_LINUX_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "include/network.h"
#include "../threading/include/threading.h"
#include "../../MQTTPacket/src/common.h"



int linux_read(Network* n, unsigned char* buffer, int len, char ensure_guarantee);
void linux_write_guaranteed(Network* n, unsigned char* buffer, int len);


#define GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(network) ((int *)(network->physical_medium))
#define HOSTNAME "localhost"
#define PORT 1883

#endif

static void release_underlying_medium(Network *n)
{
    // Close the socket
    close(*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(n)));
}


static void connect_underlying_medium_guaranteed(Network* network)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;

	int rc = SUCCESS;

	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(HOSTNAME, NULL, &hints, &result)) == 0)
	{
		struct addrinfo* res = result;

		/* prefer ip4 addresses */
		while (res)
		{
			if (res->ai_family == AF_INET)
			{
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET)
		{
			address.sin_port = htons(PORT);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else
			rc = FAILURE;

		freeaddrinfo(result);
	}

	if (rc == SUCCESS)
	{
		*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(network)) = socket(family, type, 0);
		if (*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(network)) != -1)
		{
			int opt = 1;
            while(connect(*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(network)), (struct sockaddr*)&address, sizeof(address)) != 0)
            {
                printf("Could not connect to the network ... retrying\n");
                thread_sleep(1);
            }
		}
	}
}


static void reinit_underlying_medium(int *bytes, Network *n, const char *mode)
{
    *bytes = 0;
    printf("Problem occurred at socket level while %s ... recreating socket ..\n", mode);

    release_underlying_medium(n);
    connect_underlying_medium_guaranteed(n);
    n->appInitCallback(n->appInitCallbackArg);
}


int linux_read(Network* n, unsigned char* buffer, int len, char ensure_guarantee)
{
	int bytes = 0;
    int rc = 0;

	while (bytes < len)
	{
		while(rc = recv(*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(n)), &buffer[bytes], (size_t)(len - bytes), 0) < 0)
        {
            reinit_underlying_medium(&bytes, n, "READING");
            if(ensure_guarantee == 0)
            {
                return FAILURE;
            }
        }

        // STRANGE: On Ubuntu 14.04, if "n" bytes are received successfully as one chunk, rc is 0 (and not "n") :(
        if(rc == 0)
        {
            bytes = len;
        }
        else
        {
            bytes = bytes + rc;
        }
	}

    return SUCCESS;
}


void linux_write_guaranteed(Network* n, unsigned char* buffer, int len)
{
    int bytes = 0;
    int rc = 0;

    while(bytes < len)
    {
	    while(rc = write(*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(n)), &buffer[bytes], (size_t)(len - bytes)) < 0)
        {
            reinit_underlying_medium(&bytes, n, "WRITING");
        }

        // STRANGE: On Ubuntu 14.04, if "n" bytes are sent successfully as one chunk, rc is 0 (and not "n") :(
        if(rc == 0)
        {
            bytes = len;
        }
        else
        {
            bytes = bytes + rc;
        }
    }
}


Network* get_new_network(void* (*appInitCallback)(void *arg), void *appInitCallbackArg)
{
    Network *network = (Network*)malloc(sizeof(Network));

    // Here, physical medium is a socket, and this represents the socket-id
	network->physical_medium = malloc(sizeof(int));

    // Must register the app-init-callback.
    network->appInitCallback = appInitCallback;
    network->appInitCallbackArg = appInitCallbackArg;

    // Register read-callback.
	network->read = linux_read;

    // Register write-callback.
	network->write_guaranteed = linux_write_guaranteed;

    // Connect the medium (socket).
    connect_underlying_medium_guaranteed(network);

    return network;
}


void release_network(Network *n)
{
    // Free the dynamically-allocated memory
    free(n->physical_medium);
    free(n);

    printf("Complete Network, including the underlying physical-medium.. cleaned !!!!!\n");
}
