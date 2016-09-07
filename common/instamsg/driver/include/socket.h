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




#ifndef INSTAMSG_SOCKET_COMMON
#define INSTAMSG_SOCKET_COMMON

#include "device_socket.h"

#define SOCKET			PROSTR("[SOCKET] ")
#define SOCKET_ERROR	PROSTR("[SOCKET_ERROR] ")
#define SOCKET_CONNECTION_SOLITARY_ATTEMPT_MAX_ALLOWED_TIME_SECONDS 300

#define SOCKET_TCP      PROSTR("TCP")
#define SOCKET_UDP      PROSTR("UDP")

#define SG_APN          PROSTR("sg_apn")
#define SG_USER         PROSTR("sg_user")
#define SG_PASS         PROSTR("sg_pass")
#define SG_PIN          PROSTR("sg_pin")
#define PROV_PIN        PROSTR("prov_pin")

#define SMS_FILE        PROSTR("/home/sensegrow/sms.txt")
#define SMS_TEMP_FILE   PROSTR("/home/sensegrow/sms_temp.txt")


/*
 * Global-functions callable.
 */
void init_socket(Socket *socket, const char *hostName, unsigned int port, const char *type);
void release_socket(Socket *socket);


/*
 * Must not be called directly.
 * Instead must be called as ::
 *
 *      socket->read
 *      socket->write
 */
int socket_read(Socket *socket, unsigned char* buffer, int len, unsigned char guaranteed);
int socket_write(Socket *socket, unsigned char* buffer, int len);


/*
 * Internally Used.
 * Must not be called by anyone.
 */
#if GSM_INTERFACE_ENABLED == 1
void store_sms_in_config(char *sms, char *smsConfigBuffer, int smsConfigBufferLength);
void get_latest_sms_containing_substring(Socket *socket, char *buffer, const char *prefix);
#endif
void connect_underlying_socket_medium_try_once(Socket* socket);
void release_underlying_socket_medium_guaranteed(Socket* socket);

#endif
