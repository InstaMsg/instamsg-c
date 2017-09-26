#include "./include/globals.h"
#include "./include/payload.h"

#include "../../instamsg/driver/include/log.h"
#include "../../instamsg/driver/include/instamsg.h"
#include "../../instamsg/driver/include/sg_mem.h"
#include "../../instamsg/driver/include/misc.h"
#include "../../instamsg/driver/include/watchdog.h"
#include "../../instamsg/driver/include/globals.h"
#include "../../instamsg/driver/include/hex.h"
#include "../../instamsg/driver/include/gps.h"
#include "../../instamsg/driver/include/time.h"
#include "../../instamsg/driver/include/sg_stdlib.h"

#include <string.h>

char *serialCommandUnderProcess;
unsigned int errorCase;

static char watchdogAssistant[50];
static char smallBuffer[MAX_BUFFER_SIZE / 2];

struct PortInfoArgument portInfoArgument;

#define USE_XML_FOR_PAYLOAD             0

static void get_command_under_process(char *messagebuffer, int maxbufferlength)
{
    memset(messagebuffer, 0, maxbufferlength);
    sg_sprintf(messagebuffer, "%s", serialCommandUnderProcess);
}


static void get_data_topic(char *messagebuffer, int maxbufferlength)
{
    memset(messagebuffer, 0, maxbufferlength);
    sg_sprintf(messagebuffer, "%s/data", instaMsg.clientIdComplete);
}


static void get_client_id(char *messagebuffer, int maxbufferlength)
{
    memset(messagebuffer, 0, maxbufferlength);
    strcpy(messagebuffer, instaMsg.clientIdComplete);
}


#if SEND_GPS_LOCATION == 1
static void get_geo_tag(char *messagebuffer, int maxbufferlength)
{
    memset(messagebuffer, 0, maxbufferlength);
    get_gps_sentence((unsigned char*)messagebuffer, maxbufferlength, GPS_LOCATION_SENTENCE_TYPE);
}
#endif


static void addPayloadField(char *messageBuffer,
                            const char *tag,
                            void (*func)(char *messageBuffer, int maxBufferLength))
{
    memset(watchdogAssistant, 0, sizeof(watchdogAssistant));
    strcat(watchdogAssistant, "Calculating-For-Payload ");
    strcat(watchdogAssistant, tag);

#if USE_XML_FOR_PAYLOAD == 1
    strcat(messageBuffer, "<");
    strcat(messageBuffer, tag);
    strcat(messageBuffer, ">");
#else
    strcat(messageBuffer, "\"");
    strcat(messageBuffer, tag);
    strcat(messageBuffer, "\" : \"");
#endif

    memset(smallBuffer, 0, sizeof(smallBuffer));

    watchdog_reset_and_enable(10, watchdogAssistant, 1);
    func(smallBuffer, sizeof(smallBuffer));
    watchdog_disable(NULL, NULL);

    strcat(messageBuffer, smallBuffer);

#if USE_XML_FOR_PAYLOAD == 1
    strcat(messageBuffer, "</");
    strcat(messageBuffer, tag);
    strcat(messageBuffer, ">");
#else
    strcat(messageBuffer, "\", ");
#endif
}


void pre_process_payload()
{
    errorCase = 0;

    /*
     * Now, start forming the payload ....
     */
    memset(messageBuffer, 0, sizeof(messageBuffer));

#if USE_XML_FOR_PAYLOAD == 1
    strcat(messageBuffer, "<?xml version=\"1.0\"?><datas><data_node>");
#else
    strcat(messageBuffer, "{");
#endif


    addPayloadField(messageBuffer, "topic", get_data_topic);
    addPayloadField(messageBuffer, "manufacturer", get_manufacturer);
    addPayloadField(messageBuffer, "client_id", get_client_id);



    /*
     * Modbus-Response
     */
#if USE_XML_FOR_PAYLOAD == 1
    strcat(messageBuffer, "<data><");
#else
    strcat(messageBuffer, "\"data\" : \"");
#endif
}


void add_port_info(char *buffer, void *arg)
{
    struct PortInfoArgument *portInfo = (struct PortInfoArgument *) arg;

    strcat(buffer, "\"v\" : \"1.0\", ");

    strcat(buffer, "\"port\" : {\"port_name\" : \"");
    strcat(buffer, portInfo->portName);

    strcat(buffer, "\", \"port_address\" : \"");
    {
        char small[3] = {0};

        if(strlen(portInfo->portAddress) > 0)
        {
            sg_sprintf(small, "%x", sg_atoi(portInfo->portAddress));
            addPaddingIfRequired(small, sizeof(small) - 1);
        }

        strcat(buffer, small);
    }

    strcat(buffer, "\", \"host_address\" : \"");
    strcat(buffer, portInfo->hostAddress);

    strcat(buffer, "\", \"host_port\" : \"");
    strcat(buffer, portInfo->hostPort);
    strcat(buffer, "\"}, ");
}


void add_csv_metadata_info(char *buffer, void *arg)
{
    struct CsvMetadataInfo *csvInfo = (struct CsvMetadataInfo *) arg;

    strcat(buffer, "\"v\" : \"1.0\", \"metadata\" : {\"protocol\" : \"csv\", \"file_id\" : \"");
    strcat(buffer, csvInfo->fileId);
    strcat(buffer, "\"}, ");
}


void post_process_payload(unsigned int errorCase, void (*func)(char *, void*), void *arg)
{
#if USE_XML_FOR_PAYLOAD == 1
    strcat(messageBuffer, "></data>");
#else
    strcat(messageBuffer, "\", ");
#endif


    addPayloadField(messageBuffer, "cmd", get_command_under_process);
    addPayloadField(messageBuffer, "id", get_device_uuid);
    addPayloadField(messageBuffer, "time", getTimeInDesiredFormat);
#if SEND_GPS_LOCATION == 1
    addPayloadField(messageBuffer, "geo", get_geo_tag);
#endif
    addPayloadField(messageBuffer, "offset", getTimezoneOffset);

#if 1
    func(messageBuffer, arg);
#endif

    if(errorCase == 1)
    {
        strcat(messageBuffer, "\"error_code\" : 101, \"error_msg\" : \"Response not received for polling.\", ");
    }

#if USE_XML_FOR_PAYLOAD == 1
    strcat(messageBuffer, "</data_node></datas>");
#else
    if(messageBuffer[strlen(messageBuffer) - 2] == ',')
    {
        messageBuffer[strlen(messageBuffer) - 2] = 0;
    }

    strcat(messageBuffer, "}");
#endif


    sg_sprintf(LOG_GLOBAL_BUFFER, "Sending device-data [%s]", messageBuffer);
    debug_log(LOG_GLOBAL_BUFFER);

    if(strlen(messageBuffer) > (sizeof(messageBuffer) - 10))
    {
        publish(NOTIFICATION_TOPIC,
                "DATA BUFFER LENGTH EXCEEDED",
                QOS0,
                0,
                NULL,
                MQTT_RESULT_HANDLER_TIMEOUT,
                1);

        resetDevice();
    }
    else
    {
        publishMessageWithDeliveryGuarantee(TOPIC_WEBHOOK, messageBuffer);
    }
}


void assignPortInfoToStructure(struct PortInfoArgument *portInfoArgument, char *portName, char *portAddress, char *hostAddress, char *hostPort)
{
    portInfoArgument->portName = portName;
    portInfoArgument->portAddress = portAddress;
    portInfoArgument->hostAddress = hostAddress;
    portInfoArgument->hostPort = hostPort;
}


#if (SEND_GPS_LOCATION == 1) || (SEND_GPIO_INFORMATION == 1)
static void send_special_command(char *data, char *command, char *portName, char *portAddress)
{
    serialCommandUnderProcess = command;

    pre_process_payload();
    strcat(messageBuffer, data);

    assignPortInfoToStructure(&portInfoArgument, portName, portAddress, (char*) "", (char*) "");
    post_process_payload(0, add_port_info, &portInfoArgument);
}
#endif


#if SEND_GPS_LOCATION == 1
void ioeye_send_gps_data_to_server(char *data)
{
    send_special_command(data, (char*) "GPS-INFO", (char*) PORT_NAME_GPS, (char*) "");
}
#endif


#if SEND_GPIO_INFORMATION == 1
void ioeye_send_gpio_data_to_server(char *data)
{
    send_special_command(data, (char*) "GPIO-INFO", (char*) PORT_NAME_GPIO, (char*) "");
}
#endif
