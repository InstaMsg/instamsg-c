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

#include "../include/clientnetwork.h"



int linux_read(Network*, unsigned char*, int);
int linux_write(Network*, unsigned char*, int);
void linux_disconnect(Network*);


#define GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(network) ((int *)(network->physical_medium))
#define HOSTNAME "localhost"
#define PORT 1883

#endif


int linux_read(Network* n, unsigned char* buffer, int len)
{
	int bytes = 0;
	while (bytes < len)
	{
		int rc = recv(*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(n)), &buffer[bytes], (size_t)(len - bytes), 0);
		if (rc == -1)
		{
			if (errno != ENOTCONN && errno != ECONNRESET)
			{
				bytes = -1;
				break;
			}
		}
		else
			bytes += rc;
	}
	return bytes;
}


int linux_write(Network* n, unsigned char* buffer, int len)
{
	int	rc = write(*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(n)), buffer, len);
	return rc;
}


void linux_disconnect(Network* n)
{
	close(*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(n)));
}


static int ConnectNetwork(Network* network)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	int rc = -1;
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
			rc = -1;

		freeaddrinfo(result);
	}

	if (rc == 0)
	{
		*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(network)) = socket(family, type, 0);
		if (*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(network)) != -1)
		{
			int opt = 1;
			rc = connect(*(GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(network)), (struct sockaddr*)&address, sizeof(address));
		}
	}

	return rc;
}


Network* get_new_network()
{
    Network *network = (Network*)malloc(sizeof(Network));

    // Here, physical medium is a socket, and this represents the socket-id
	network->physical_medium = malloc(sizeof(int));

	network->mqttread = linux_read;
	network->mqttwrite = linux_write;
	network->disconnect = linux_disconnect;

    ConnectNetwork(network);

    return network;
}

void release_network(Network *n)
{
    free(n->physical_medium);
    free(n);

    printf("Complete Network, including the underlying physical-medium.. cleaned !!!!!\n");
}
