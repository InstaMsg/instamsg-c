#include "device_defines.h"

#if DEFAULT_SIMCOM_SOCKET_ENABLED == 1

#include "device_socket.h"

extern volatile char *responseBuffer;
extern volatile unsigned char readResponse;
extern volatile char *response_delimiter;

#define CIRCULAR_BUFFER_SIZE 500

void simcom_get_latest_sms_containing_substring(SG_Socket *socket, char *buffer, const char *substring);
void simcom_connect_underlying_socket_medium_try_once(SG_Socket* socket);
int simcom_socket_read(SG_Socket* socket, unsigned char* buffer, int len, unsigned char guaranteed);
int simcom_socket_write(SG_Socket* socket, unsigned char* buffer, int len);
void simcom_release_underlying_socket_medium_guaranteed(SG_Socket* socket);
void simcom_load_client_certificate_into_buffer(char *cert_buffer, int maxLength);
void simcom_save_client_certificate_from_buffer(char *cert_buffer);
void simcom_load_client_private_key_into_buffer(char *private_key_buffer, int maxLength);
void simcom_save_client_private_key_from_buffer(char *private_key_buffer);

void reset_circular_buffer();
void add_data_to_circular_buffer(unsigned char c);
short remove_unwanted_line_with_prefix(char *usefulOutput, char *prefix);


extern volatile unsigned char timeSyncedViaExternalResources;
#define WAIT_FOR_SIMCOM_MODEM_RESPONSE                                                                                                  \
    {                                                                                                                                   \
        responseBuffer = usefulOutput;                                                                                                  \
        response_delimiter = (volatile char*)delimiter;                                                                                 \
                                                                                                                                        \
        readResponse = 1;                                                                                                               \
                                                                                                                                        \
        while( (readResponse == 1) && (time_fine_for_time_limit_function() == 1) )                                                      \
        {                                                                                                                               \
            serial_poller_func();                                                                                                       \
        }                                                                                                                               \
                                                                                                                                        \
        /*                                                                                                                              \
         * If the complete response was not received, and we reach till here (case of watchdog-not-resetting-device-immediately),       \
         * then discard any partial bytes.                                                                                              \
         */                                                                                                                             \
        if(readResponse == 1)                                                                                                           \
        {                                                                                                                               \
            usefulOutput[0] = 0;                                                                                                        \
        }                                                                                                                               \
                                                                                                                                        \
                                                                                                                                        \
        /*                                                                                                                              \
         * Remove the URCs received asynchronously from the modem.                                                                      \
         */                                                                                                                             \
        while(1)                                                                                                                        \
        {                                                                                                                               \
            short removed = 0;                                                                                                          \
            removed = removed + remove_unwanted_line_with_prefix(usefulOutput, "+CIPRXGET: 1,");                                        \
                                                                                                                                        \
            {                                                                                                                           \
                short reply = remove_unwanted_line_with_prefix(usefulOutput, "*PSUTTZ:");                                               \
                if(reply == 1)                                                                                                          \
                {                                                                                                                       \
                    sg_sprintf(LOG_GLOBAL_BUFFER, "^^^^^^^^^^^^^^^^^^^ *PSUTTZ received .. time synced !!! ^^^^^^^^^^^^^^^^^^^ ");      \
                    info_log(LOG_GLOBAL_BUFFER);                                                                                        \
                                                                                                                                        \
                    timeSyncedViaExternalResources = 1;                                                                                 \
                }                                                                                                                       \
                                                                                                                                        \
                removed = removed + reply;                                                                                              \
            }                                                                                                                           \
                                                                                                                                        \
            removed = removed + remove_unwanted_line_with_prefix(usefulOutput, "DST:");                                                 \
            removed = removed + remove_unwanted_line_with_prefix(usefulOutput, "+CIEV:");                                               \
            removed = removed + remove_unwanted_line_with_prefix(usefulOutput, "*PSNWID:");                                             \
            removed = removed + remove_unwanted_line_with_prefix(usefulOutput, "+CTZV:");                                               \
                                                                                                                                        \
            if(removed == 0)                                                                                                            \
            {                                                                                                                           \
                break;                                                                                                                  \
            }                                                                                                                           \
        }                                                                                                                               \
    }


#endif
