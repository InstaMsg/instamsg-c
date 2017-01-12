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



#ifndef INSTAMSG_MISC_COMMON
#define INSTAMSG_MISC_COMMON

/*
 * Need to be implemented by the device.
 */
void resetDevice();
void bootstrapInit();
void get_client_session_data(char *messageBuffer, int maxBufferLength);
void get_client_metadata(char *messageBuffer, int maxBufferLength);
void get_network_data(char *messageBuffer, int maxBufferLength);
void get_manufacturer(char *messagebuffer, int maxbufferlength);
void get_device_uuid(char *buffer, int maxbufferlength);
void get_device_ip_address(char *buffer, int maxbufferlength);


/*
 * Need to be implemented by the app.
 */
void release_app_resources();


/*
 * INTERNAL.
 */
#if GSM_INTERFACE_ENABLED == 1
#else
void get_prov_pin_for_non_gsm_devices(char *buffer, int maxbufferlength);
#endif

void waitBeforeReboot();
void exitApp(unsigned char waitForReboot);

#endif
