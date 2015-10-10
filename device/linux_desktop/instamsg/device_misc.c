#include <signal.h>
#include <string.h>

#ifdef DEBUG_MODE
#include "../../../../common/instamsg/driver/include/globals.h"
#endif


/*
 * Utility-function that reboots the device.
 */
void rebootDevice()
{
    info_log("Rebooting the system.");
    system("/sbin/reboot");
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
void get_device_uuid(char *buffer, int maxbufferlength)
{
    /*
     * We assume that """EVERY""" Linux-Desktop device-type has an interface called "eth0",
     * with a unique MAC-address.
     *
     * We use that, in combination with LINUX-DESKTOP:ETH0:MAC as the prefix.
     */
    int fd;
	struct ifreq ifr;
	char *iface = "eth0";
	unsigned char *mac;
    int i;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	ioctl(fd, SIOCGIFHWADDR, &ifr);

	close(fd);
	mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

    strcat(buffer, "LINUX-DESKTOP:ETH0:MAC:");
    for(i = 0; i < 6; i++)
    {
        char hex[3] = {0};

        sg_sprintf(hex, "%x", mac[i]);
        if(strlen(hex) == 1)
        {
            strcat(buffer, "0");
        }
        strcat(buffer, hex);
        strcat(buffer, "-");
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
