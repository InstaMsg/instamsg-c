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



#include <signal.h>
#include <string.h>

#include <stdlib.h>

#include "../../../common/instamsg/driver/include/globals.h"



/*
 * Utility-function that resets the device, bringing it into a clean, fresh state.
 */
void resetDevice()
{
    exit(1);
}


/*
 * This method does the initialization, that is needed on a global-basis.
 * All code in this method runs right at the beginning (followed by logger-initialization).
 *
 * THIS METHOD IS GENERALLY EXPECTED TO BE EMPTY, AS THE INITIALIZATION SHOULD HAPPEN ON
 * PER-MODULE BASIS. ONLY IF SOMETHING IS REALLY NEEDED RIGHT AT THE START (WHICH IS THEN
 * POTENTIALLY USED BY MORE THAN ONE MODULE), SHOULD SOMETHING BE REQUIRED TO COME HERE.
 */
void bootstrapInit()
{
    /* VERY IMPORTANT: If this is not done, the "write" on an invalid socket will cause program-crash */
    signal(SIGPIPE, SIG_IGN);
}


/*
 * This method returns the client-network-data, in simple JSON form, of type ::
 *
 * {'method' : 'value', 'ip_address' : 'value', 'antina_status' : 'value', 'signal_strength' : 'value'}
 */
void get_client_session_data(char *messageBuffer, int maxBufferLength)
{
}


/*
 * This method returns the client-network-data, in simple JSON form, of type ::
 *
 * {'imei' : 'value', 'serial_number' : 'value', 'model' : 'value', 'firmware_version' : 'value', 'manufacturer' : 'value', 'client_version' : 'value'}
 */
void get_client_metadata(char *messageBuffer, int maxBufferLength)
{
}


/*
 * This method returns the client-network-data, in simple JSON form, of type ::
 *
 * {'antina_status' : 'value', 'signal_strength' : 'value'}
 */
void get_network_data(char *messageBuffer, int maxBufferLength)
{
}


/*
 * This method gets the device-manufacturer.
 */
void get_manufacturer(char *messagebuffer, int maxbufferlength)
{
}


/*
 * This method returns the univerally-unique-identifier for this device.
 */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

const char *IFACE = "eth0";

void get_device_uuid(char *buffer, int maxbufferlength)
{
    /*
     * We assume that """EVERY""" Linux-Desktop device-type has an interface called "eth0",
     * with a unique MAC-address.
     *
     * We use that, in combination with LINUX-DESKTOP:WLAN0:MAC as the prefix.
     */
    int fd;
	struct ifreq ifr;
	unsigned char *mac;
    int i;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, IFACE, IFNAMSIZ - 1);

	ioctl(fd, SIOCGIFHWADDR, &ifr);

	close(fd);
	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

    strcat(buffer, "");
    for(i = 0; i < 6; i++)
    {
        char hex[3] = {0};

        sg_sprintf(hex, "%x", mac[i]);
        if(strlen(hex) == 1)
        {
            strcat(buffer, "0");
        }
        strcat(buffer, hex);
        strcat(buffer, ":");
    }
    /*
     * Strip the last "-"
     */
    buffer[strlen(buffer) - 1] = 0;

#ifdef DEBUG_MODE
    if(strlen(USER_DEVICE_UUID) > 0)
    {
        memset(buffer, 0, maxbufferlength);
        strcpy(buffer, USER_DEVICE_UUID);
    }
#endif
}


/*
 * This method returns the ip-address of this device.
 */
#include <ifaddrs.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
void get_device_ip_address(char *buffer, int maxbufferlength)
{
    struct ifaddrs *ifAddrStruct = NULL;
    struct ifaddrs * ifa = NULL;
    void *tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    for(ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            if(strcmp(ifa->ifa_name, IFACE) == 0)
            {
                tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, tmpAddrPtr, buffer, INET_ADDRSTRLEN);
            }
            else
            {
                continue;
            }
        }
    }

    if(ifAddrStruct != NULL)
        freeifaddrs(ifAddrStruct);
}


#if GSM_INTERFACE_ENABLED == 1
#else
/*
 * This method returns the provisioning-pin for this device.
 */
void get_prov_pin_for_non_gsm_devices(char *buffer, int maxbufferlength)
{
    strcpy(buffer, "test");
}
#endif

