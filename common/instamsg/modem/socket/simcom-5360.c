#include "device_defines.h"

#if DEFAULT_SIMCOM_5360_SOCKET_ENABLED == 1

#include "./simcom-5360.h"

#include "../driver/include/globals.h"
#include "../driver/include/at.h"
#include "../driver/include/log.h"
#include "../driver/include/sg_mem.h"
#include "../driver/include/sg_stdlib.h"
#include "../driver/include/misc.h"
#include "../driver/include/socket.h"


char modemReceiveBuffer[MAX_BUFFER_SIZE];
volatile int modemReceiveBytesSoFar;

static char smallBuffer[100];
static char receiveBuffer[MAX_BUFFER_SIZE];

static volatile unsigned char errorObtained;

static volatile unsigned char bytesStartReading;
static volatile unsigned char *bytesFromServerBuffer;
static volatile int bytesActuallyRead;

volatile char *responseBuffer;
volatile unsigned char readResponse;
volatile char *response_delimiter;

volatile unsigned int writeIndex;
volatile unsigned int readIndex;
volatile unsigned char circularBuffer[2 * CIRCULAR_BUFFER_SIZE];

static unsigned char returnSingleCharacter()
{
    unsigned char c;

    while(circularBuffer[readIndex] == INVALID_DATA)
    {
        startAndCountdownTimer(1, 0);
    }

    c = circularBuffer[readIndex + 1];
    circularBuffer[readIndex] = INVALID_DATA;

    readIndex = readIndex + 2;
    if(readIndex == (2 * CIRCULAR_BUFFER_SIZE))
    {
        readIndex = STARTING_INDEX;
    }

    return c;
}


static unsigned char isError(const char *atResponse)
{
    if(strstr(atResponse, "ERROR\r\n") == NULL)
    {
        return 0;
    }

    return 1;
}


static unsigned char readNextChar()
{
    unsigned char c = returnSingleCharacter();

    *(modemReceiveBuffer + modemReceiveBytesSoFar) = c;
    modemReceiveBytesSoFar++;

    return c;
}


static void read_till_newline()
{
    while(1)
    {
        unsigned char c = readNextChar();
        if(c == '\n')
        {
            break;
        }
    }
}


static void reset_modem_receive_buffer()
{
    memset(modemReceiveBuffer, 0, sizeof(modemReceiveBuffer));
    modemReceiveBytesSoFar = 0;
}

void reset_circular_buffer()
{
	{
        int i = 0;
		for(i = 0; i < (2 * CIRCULAR_BUFFER_SIZE); i++)
		{
			circularBuffer[i] = INVALID_DATA;
		}

		writeIndex = STARTING_INDEX;
		readIndex = STARTING_INDEX;

		reset_modem_receive_buffer();
	}
}



#define SOCKET_CONNECTION_DELIM     "CONNECT"
#define BYTES_POLL_RESP_HEADER      "+CIPRXGET: 2,"

static void check_if_output_desired_and_available()
{
    int lengthOfDelimiter = strlen((char*)response_delimiter);
    int bytesReadTillThisIteration = modemReceiveBytesSoFar;

    if(1)
    {
        if(readResponse == 1)
        {
            unsigned char okToCheckForDelimiter = 0;
            if( (*(response_delimiter + lengthOfDelimiter - 1) == '\n') || (strcmp((char*)response_delimiter, SOCKET_CONNECTION_DELIM) == 0) )
            {
                if(modemReceiveBytesSoFar > 0)
                {
                    if(*(modemReceiveBuffer + modemReceiveBytesSoFar - 1) == '\n')
                    {
                        okToCheckForDelimiter = 1;
                    }
                }
            }
            else
            {
                if(modemReceiveBytesSoFar >= lengthOfDelimiter)
                {
                    okToCheckForDelimiter = 1;
                }
            }

            if(okToCheckForDelimiter == 1)
            {
                char *ptr = (char*)sg_memnmem(modemReceiveBuffer, (char*)response_delimiter, modemReceiveBytesSoFar, lengthOfDelimiter);
                if(ptr != NULL)
                {
                    /*
                     * Send the bytes so-far to response-buffer.
                     */
                    memcpy((void*)responseBuffer, modemReceiveBuffer, modemReceiveBytesSoFar);
                    reset_modem_receive_buffer();

                    if(strcmp((char*)response_delimiter, BYTES_POLL_RESP_HEADER) == 0)
                    {
                        /*
                         * Socket-Bytes-Reading Case
                         */
                        read_till_newline();

                        /*
                         * Send the bytes till newline.
                         */
                        memcpy((char*)responseBuffer + bytesReadTillThisIteration, modemReceiveBuffer, modemReceiveBytesSoFar);


                        {
                            char *bytesPollRespHeader = (char*)sg_memnmem((char*)responseBuffer, BYTES_POLL_RESP_HEADER,
                                                        modemReceiveBytesSoFar + bytesReadTillThisIteration, strlen(BYTES_POLL_RESP_HEADER));
                            if(bytesPollRespHeader != NULL)
                            {
                                char bytesAvailable[5] = {0};
                                int i = 0;
                                int bytesAvailableFromServer = 0;

                                get_nth_token_thread_safe(bytesPollRespHeader, ',', 3, bytesAvailable, 1);

                                bytesAvailableFromServer = sg_atoi(bytesAvailable);
                                for(i = 0; i < bytesAvailableFromServer; i++)
                                {
                                    char c = returnSingleCharacter();
                                    *(bytesFromServerBuffer + i) = c;
                                    bytesActuallyRead++;
                                }

                                bytesStartReading = 0;
                            }
                        }

                        /*
                         * Remove the additional bytes (after all, PROTOCOL IS GOD).
                         */
                        memset((char*)responseBuffer + bytesReadTillThisIteration, 0, modemReceiveBytesSoFar);
                        reset_modem_receive_buffer();
                    }

                    readResponse = 0;
                }
            }
        }
    }
}


void serial_poller_func()
{
    readNextChar();
    check_if_output_desired_and_available();
}


short remove_unwanted_line_with_prefix(char *usefulOutput, const char *prefix)
{
    char *prefixPointer = strstr(usefulOutput, prefix);
    if(prefixPointer != NULL)
    {
        char *newLinePointer = strstr(prefixPointer, "\n");
        if(newLinePointer != NULL)
        {
            /* xyzPREFIXabcdef\nrestofstuff */

            short lenPrefix = strlen(prefixPointer);            // 24
            short lenNewLine = strlen(newLinePointer);          // 12
            short lengthToErase = lenPrefix - lenNewLine + 1;   // 13

            if(lengthToErase < lenPrefix)
            {
                memcpy(prefixPointer, newLinePointer + 1, lenNewLine - 1);
            }

            prefixPointer[lenNewLine - 1] = 0;
            return 1;
        }
        else
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "No newline pointer .. catastrophe awaits !!");
            error_log(LOG_GLOBAL_BUFFER);

            resetDevice();
        }
    }

    return 0;
}


#define MODEM_SLEEP_INTERVAL 3

#define MODEM           "[MODEM] "
#define COMMAND         "[COMMAND %u] "
#define MODEM_COMMAND   "[MODEM_INIT_COMMAND %u] "
#define MODEM_SOCKET    "[MODEM_SOCKET_COMMAND %u] "

typedef struct SocketInitCommands SocketInitCommands;

struct SocketInitCommands
{
    /*
     * The command to run.
     */
    const char *command;

    /*
     * The command-response delimiter.
     */
    const char *delimiter;

    /*
     * For logging purposes.
     */
    const char *logInfoCommand;

    /*
     * Assuming that there can be a maximum of 5 such strings.
     */
    const char *successStrings[5];

    /*
     * If the command-output does not contain any of the expected-strings, this command
     * needs to be run for rectification.
     *
     * Ultimately, after running this command (1 or more times), the output of "command"
     * must contain one of the expected-strings.
     */
    const char *commandInCaseNoSuccessStringPresent;

    /*
     * A flag, so that we run the rectification-command just once.
     */
    unsigned char rectificationCommandTriedOnceAlready;
};
SocketInitCommands commands[8];


static int runBatchCommands(const char *batchName, unsigned char giveModemSleep)
{
    int i, j, passed, failed;

    i = 0;
    while(1)
    {
        if(commands[i].command != NULL)
        {
            commands[i].rectificationCommandTriedOnceAlready = 0;
            i++;
        }
        else
        {
            break;
        }
    }

start_commands:

    if(giveModemSleep == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Giving modem [%u] seconds, before checking all the settings and statuses", MODEM_SLEEP_INTERVAL);
        info_log(LOG_GLOBAL_BUFFER);

        startAndCountdownTimer(MODEM_SLEEP_INTERVAL, 1);
    }

    passed = 0;
    failed = 0;
    i = 0;

    while(1)
    {
        startAndCountdownTimer(1, 0);

        if(commands[i].command == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "\n\n[%s] TOTAL COMMANDS: [%u], PASSED: [%u], FAILED: [%u]\n\n", batchName, i, passed, failed);
            info_log(LOG_GLOBAL_BUFFER);

            break;
        }

        sg_sprintf(LOG_GLOBAL_BUFFER, "\n\n");
        info_log(LOG_GLOBAL_BUFFER);

        sg_sprintf(LOG_GLOBAL_BUFFER, COMMAND "Running [%s] for \"%s\"", i + 1, commands[i].command, commands[i].logInfoCommand);
        info_log(LOG_GLOBAL_BUFFER);

        run_simple_at_command_and_get_output(commands[i].command, strlen(commands[i].command),
                                             messageBuffer, sizeof(messageBuffer), commands[i].delimiter, 1, 0);

        while(1)
        {
            j = 0;

            while(1)
            {

                if(commands[i].successStrings[j] == NULL)
                {
                    if((commands[i].commandInCaseNoSuccessStringPresent != NULL) &&
                       (commands[i].rectificationCommandTriedOnceAlready == 0))
                    {
                        sg_sprintf(LOG_GLOBAL_BUFFER, COMMAND "Initial Check for \"%s\" Failed.. trying to rectify with [%s]",
                                   i + 1, commands[i].logInfoCommand, commands[i].commandInCaseNoSuccessStringPresent);
                        info_log(LOG_GLOBAL_BUFFER);

                        commands[i].rectificationCommandTriedOnceAlready = 1;
                        run_simple_at_command_and_get_output(
                                commands[i].commandInCaseNoSuccessStringPresent, strlen(commands[i].commandInCaseNoSuccessStringPresent),
                                messageBuffer, sizeof(messageBuffer), commands[i].delimiter, 1, 0);

                        goto start_commands;
                    }
                    else
                    {
                        sg_sprintf(LOG_GLOBAL_BUFFER, COMMAND "\"%s\" Failed :(", i + 1, commands[i].logInfoCommand);
                        info_log(LOG_GLOBAL_BUFFER);

                        failed++;
                        break;
                    }
                }


                {
                    char *token, *temp;

                    RESET_GLOBAL_BUFFER;
                    strcpy((char*)GLOBAL_BUFFER, commands[i].successStrings[j]);

                    temp = (char*)GLOBAL_BUFFER;
                    while(1)
                    {
                        token = strtok(temp, "&");
                        if(token == NULL)
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER, COMMAND "\"%s\" Passed", i + 1, commands[i].logInfoCommand);
                            info_log(LOG_GLOBAL_BUFFER);

                            passed++;

                            goto continue_with_next_command;
                        }

                        if(strstr(messageBuffer, token) != NULL)
                        {
                            sg_sprintf(LOG_GLOBAL_BUFFER, COMMAND "Found [%s] in output", i + 1, token);
                            info_log(LOG_GLOBAL_BUFFER);
                        }
                        else
                        {
                            goto continue_with_next_success_string;
                        }

                        /* For next call to strtok */
                        temp = NULL;
                    }
                }

continue_with_next_success_string:
                j++;
            }

            break;
        }


continue_with_next_command:
        i++;
    }

    if(failed == 0)
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}


static unsigned char setUpModemDone;

static int setUpModemMinimal(SG_Socket *socket)
{
    /*
     * Prepare-init-commands.
     */
    commands[0].command = "AT+IPR=9600\r";
    commands[0].delimiter = OK_DELIMITER;
    commands[0].logInfoCommand = "Set-Baud-Rate";
    commands[0].successStrings[0] = OK_DELIMITER;
    commands[0].successStrings[1] = NULL;
    commands[0].commandInCaseNoSuccessStringPresent = NULL;

    /*
     */
    commands[1].command = "AT&W\r";
    commands[1].delimiter = OK_DELIMITER;
    commands[1].logInfoCommand = "Persist-Baud-Rate";
    commands[1].successStrings[0] = OK_DELIMITER;
    commands[1].successStrings[1] = NULL;
    commands[1].commandInCaseNoSuccessStringPresent = NULL;

    /*
     */
    commands[2].command = "ATE0&W\r";
    commands[2].delimiter = OK_DELIMITER;
    commands[2].logInfoCommand = "Stop-Echo";
    commands[2].successStrings[0] = OK_DELIMITER;
    commands[2].successStrings[1] = NULL;
    commands[2].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[3].command = "AT+CPIN?\r";
    commands[3].delimiter = OK_DELIMITER;
    commands[3].logInfoCommand = "SIM-PIN-Ready";
    commands[3].successStrings[0] = "READY";
    commands[3].successStrings[1] = NULL;
    commands[3].commandInCaseNoSuccessStringPresent = NULL;

    /*
     */
    commands[4].command = NULL;
    return runBatchCommands("MODEM-SETUP-MINIMAL", 1);
}


static int checkGsmSyncEnabled(SG_Socket *socket)
{
    commands[0].command = "AT+CLTS?\r";
    commands[0].delimiter = OK_DELIMITER;
    commands[0].logInfoCommand = "Check-GSM-Sync-CLTS";
    commands[0].successStrings[0] = "+CLTS: 1";
    commands[0].successStrings[1] = NULL;
    commands[0].commandInCaseNoSuccessStringPresent = NULL;

    commands[1].command = "AT+IPR?\r";
    commands[1].delimiter = OK_DELIMITER;
    commands[1].logInfoCommand = "Check-GSM-Sync-IPR";
    commands[1].successStrings[0] = "+IPR: 9600";
    commands[1].successStrings[1] = NULL;
    commands[1].commandInCaseNoSuccessStringPresent = NULL;

    /*
     */
    commands[2].command = NULL;
    return runBatchCommands("CHECK-GSM-SYNC-ENABLED", 1);
}


static int enableGsmSync(SG_Socket *socket)
{
    commands[0].command = "AT+CLTS=1\r";
    commands[0].delimiter = OK_DELIMITER;
    commands[0].logInfoCommand = "Enable-GSM-Sync-CLTS";
    commands[0].successStrings[0] = OK_DELIMITER;
    commands[0].successStrings[1] = NULL;
    commands[0].commandInCaseNoSuccessStringPresent = NULL;

    /*
     */
    commands[1].command = "AT&W\r";
    commands[1].delimiter = OK_DELIMITER;
    commands[1].logInfoCommand = "Persist-GSM-Sync";
    commands[1].successStrings[0] = OK_DELIMITER;
    commands[1].successStrings[1] = NULL;
    commands[1].commandInCaseNoSuccessStringPresent = NULL;

    /*
     */
    commands[2].command = NULL;
    return runBatchCommands("ENABLE-AND-PERSIST-GSM-SYNC", 1);
}


static int setUpModem(SG_Socket *socket)
{
    int rc = FAILURE;

    if(setUpModemDone == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "%s\"setUpModem\" step already done successfully, not re-doing", SOCKET);
        info_log(LOG_GLOBAL_BUFFER);

        return SUCCESS;
    }

    while(setUpModemMinimal(socket) != SUCCESS)
    {
    }

    if(checkGsmSyncEnabled(socket) != SUCCESS)
    {
        while(enableGsmSync(socket) != SUCCESS)
        {
        }

        sg_sprintf(LOG_GLOBAL_BUFFER, "GSM-Sync-Feature has been enabled, we need to reset once ...");
        info_log(LOG_GLOBAL_BUFFER);

        resetDevice();
    }

    /*
     */
    commands[0].command = "AT+CREG?\r";
    commands[0].delimiter = OK_DELIMITER;
    commands[0].logInfoCommand = "SIM-Registered-To-Network";
    commands[0].successStrings[0] = "0,1";
    commands[0].successStrings[1] = "0,5";
    commands[0].successStrings[2] = NULL;
    commands[0].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[1].command = "AT+CGATT?\r";
    commands[1].delimiter = OK_DELIMITER;
    commands[1].logInfoCommand = "GPRS-Attachment-State";
    commands[1].successStrings[0] = "+CGATT: 1";
    commands[1].successStrings[1] = NULL;
#if DISABLE_EXPLICIT_PS_ATTACHMENT == 1
    commands[1].commandInCaseNoSuccessStringPresent = NULL;
#else
    commands[1].commandInCaseNoSuccessStringPresent = "AT+CGATT=1\r";
#endif


    /*
     */
    commands[2].command = "AT+CIPMUX?\r";
    commands[2].delimiter = OK_DELIMITER;
    commands[2].logInfoCommand = "Enable-Multi-Connection";
    commands[2].successStrings[0] = "+CIPMUX: 1";
    commands[2].successStrings[1] = NULL;
    commands[2].commandInCaseNoSuccessStringPresent = "AT+CIPMUX=1\r";



    /*
     */
    commands[3].command = "AT+CIPRXGET?\r";
    commands[3].delimiter = OK_DELIMITER;
    commands[3].logInfoCommand = "Enable-Data-Sync-Read";
    commands[3].successStrings[0] = "+CIPRXGET: 1";
    commands[3].successStrings[1] = NULL;
    commands[3].commandInCaseNoSuccessStringPresent = "AT+CIPRXGET=1\r";


    /*
     */
    commands[4].command = "AT+CGDCONT?\r";
    commands[4].delimiter = OK_DELIMITER;
    commands[4].logInfoCommand = "GPRS-Context-Correctness";
    commands[4].successStrings[0] = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[4].successStrings[0] == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory for successStrings[1] while testing for GPRS-context");
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }
    else
    {
        sg_sprintf((char*) (commands[4].successStrings[0]),"1,\"IP\",\"%s\"", socket->gsmApn);
    }
    commands[4].successStrings[1] = NULL;
    commands[4].commandInCaseNoSuccessStringPresent = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[4].commandInCaseNoSuccessStringPresent == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory for commandInCaseNoSuccessStringPresent while testing for GPRS-context");
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }
    else
    {
        sg_sprintf((char*) (commands[4].commandInCaseNoSuccessStringPresent), "AT+CGDCONT=1,\"IP\",\"%s\"\r",
                   socket->gsmApn);
    }

    /*
     */
    commands[5].command = NULL;
    rc = runBatchCommands("MODEM-SETUP", 1);


exit:
    if(commands[4].commandInCaseNoSuccessStringPresent)
        sg_free((char*) (commands[4].commandInCaseNoSuccessStringPresent));

    if(commands[4].successStrings[0])
        sg_free((char*) (commands[4].successStrings[0]));


    if(rc == SUCCESS)
    {
        setUpModemDone = 1;
    }

    return rc;
}


#if GSM_INTERFACE_ENABLED == 1
/*
 * This method returns the *****LATEST****** sms, which contains the desired substring, while the following returns 1 ::
 *
 *                                      time_fine_for_time_limit_function()
 *
 * Note that "{" are sometimes not processed correctly by some SIMs, so a prefix-match (which
 * otherwise is a stronger check) is not being done.
 *
 * Please note that this method is called by Instamsg-application, *****BEFORE***** calling
 * "connect_underlying_socket_medium_try_once".
 */
#define SMS_MODE_SETTER_COMMAND         "AT+CMGF=1\r"
static char tempSmsBuffer[400];

void simcom_5360_get_latest_sms_containing_substring(SG_Socket *socket, char *buffer, const char *substring)
{
    int smsIndex = 1;



    /*
     * The modem must be set-up properly (SIM in place, etc.) for us to retrieve the sms.
     */
    while(setUpModem(socket) == FAILURE)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Retrying setting up modem, as we need the bootstrap sms ..");
        info_log(LOG_GLOBAL_BUFFER);
    }

    /*
     * Enable retrieving of SMS.
     */
    while(1)
    {
        run_simple_at_command_and_get_output(SMS_MODE_SETTER_COMMAND, strlen(SMS_MODE_SETTER_COMMAND),
                                             messageBuffer, sizeof(messageBuffer), OK_DELIMITER, 1, 0);
        if(isError(messageBuffer) == 0)
        {
            break;
        }
    }

    while(1)
    {
        unsigned char readingPayload = 0;
        char *tempBuffer = tempSmsBuffer;
        int bufferIndex = 0;

        /*
         * Empty the temporary-buffer...
         */
        tempBuffer[0] = 0;

        memset(smallBuffer, 0, sizeof(smallBuffer));
        sg_sprintf(smallBuffer, "AT+CMGR=%u\r", smsIndex);

        sg_sprintf(LOG_GLOBAL_BUFFER, "\n\nScanning SMS [%u] for Provisioning-Params", smsIndex);
        info_log(LOG_GLOBAL_BUFFER);

        run_simple_at_command_and_get_output(smallBuffer, strlen(smallBuffer), messageBuffer, sizeof(messageBuffer), OK_DELIMITER, 1, 0);
        if(isError(messageBuffer) == 1)
        {
            /*
             * We stop scanning further SMSes.. if an ERROR-identifier was received as the command-output...
             */
            break;
        }

#if 0
        /*
         * Code for testing when the sms is not returned.
         */
        messageBuffer[0] = 0;
#endif

        if(messageBuffer[0] == 0)
        {
            /*
             * We did not get an error, but we did not receive a response as well, in this case too, break...
             * This is an undesirable case, so mark as nothing read from sms-storage,
             */
            buffer[0] = 0;

            sg_sprintf(LOG_GLOBAL_BUFFER,
                       "We probably hit the Hang-While-SMS-Reading stage, so failing the operation of reading sms from storage");
            error_log(LOG_GLOBAL_BUFFER);

            break;
        }
        else
        {
            /*
             * or the SMS-payload is empty.
             */
            unsigned char newLineStart = 0;
            int i;
            const char *metadataPrefix = "+CMGR:";

            for(i = 0; i < strlen(messageBuffer); i++)
            {
                if(messageBuffer[i] == '\n')
                {
                    if(readingPayload == 1)
                    {
                        /*
                         * We have reached a newline while reading payload.. so we are done.
                         */
                        break;
                    }
                    else
                    {
                        /*
                         * Check if this line had the required prefix.. if yes.. move to reading-payload mode.
                         */
                        if(1)
                        {
                            if(strncmp(messageBuffer + newLineStart, metadataPrefix, strlen(metadataPrefix)) == 0)
                            {
                                readingPayload = 1;
                            }
                        }
                    }

                    newLineStart = i + 1;
                }
                else
                {
                    if(readingPayload == 1)
                    {
                        /*
                        * We are in readingPayload-mode, keep reading..
                        */
                        tempBuffer[bufferIndex] = messageBuffer[i];
                        bufferIndex++;
                    }
                }
            }
        }

        smsIndex++;

        /*
         * Strip in trailing '\r', if any.
         */
        bufferIndex--;
        while(1)
        {
            if(tempBuffer[bufferIndex] == '\r')
            {
                tempBuffer[bufferIndex] = 0;
                bufferIndex--;
            }
            else
            {
                break;
            }
        }

        /*
         * Null-Terminate the temporary-buffer.
         */
        tempBuffer[bufferIndex + 1] = 0;

        sg_sprintf(LOG_GLOBAL_BUFFER, "SMS-Payload = [%s]", tempBuffer);
        info_log(LOG_GLOBAL_BUFFER);


        /*
         * If the substring is present, we copy from temporary-buffer to actual buffer.
         */
        if(strstr(tempBuffer, substring) != NULL)
        {
            int j;
            for(j = 0; j < strlen(tempBuffer); j++)
            {
                buffer[j] = tempBuffer[j];
            }

            /*
             * Null-Terminate the actual-buffer.
             */
            buffer[j] = 0;
        }

        /*
         * If no payload was read in this cycle, means there are no more SMSes to scan.
         */
        if(readingPayload == 0)
        {
            break;
        }
    }

    sg_sprintf(LOG_GLOBAL_BUFFER, "\n\n\n\nFinished scanning SMSes..");
    info_log(LOG_GLOBAL_BUFFER);
}
#endif

static unsigned char bringUpWirelessOrResetDone;
static int bringUpWirelessOrReset(SG_Socket *socket)
{
    int rc = FAILURE;

    if(bringUpWirelessOrResetDone == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "%s\"bringUpWirelessOrReset\" step already done successfully, not re-doing", SOCKET);
        info_log(LOG_GLOBAL_BUFFER);

        return SUCCESS;
    }

    commands[0].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[0].command == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory for AT+CSTT");
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }

    sg_sprintf((char*) (commands[0].command), "AT+CSTT=\"%s\"\r", socket->gsmApn);
    commands[0].delimiter = OK_DELIMITER;
    commands[0].logInfoCommand = "APN-Params";
    commands[0].successStrings[0] = OK_DELIMITER;
    commands[0].successStrings[1] = NULL;
    commands[0].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[1].command = "AT+CIICR\r";
    commands[1].delimiter = OK_DELIMITER;
    commands[1].logInfoCommand = "Bring-Up-Wireless-Connection";
    commands[1].successStrings[0] = OK_DELIMITER;
    commands[1].successStrings[1] = NULL;
    commands[1].commandInCaseNoSuccessStringPresent = NULL;


    commands[2].command = NULL;
    rc = runBatchCommands("BRING-UP-WIRELESS", 0);

exit:
    if(commands[0].command)
        sg_free((char*) (commands[0].command));


    if(rc == FAILURE)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not bring up wireless, nothing to live for ...");
        info_log(LOG_GLOBAL_BUFFER);

        resetDevice();
    }

    if(rc == SUCCESS)
    {
        bringUpWirelessOrResetDone = 1;
    }

    return rc;
}


static unsigned int nextConnectionNumber;
static int setUpModemSocketUDP(SG_Socket *socket)
{
    int rc = FAILURE;

    /*
     */
    commands[0].command = "AT+CIFSR\r";
    commands[0].delimiter = "\r\n";
    commands[0].logInfoCommand = "Get-IP-Address-Or-Connect-Will-Always-Fail";
    commands[0].successStrings[0] = "\r\n";
    commands[0].successStrings[1] = NULL;
    commands[0].commandInCaseNoSuccessStringPresent = NULL;

    /*
     */
    commands[1].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[1].command == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, MODEM_SOCKET "Could not allocate memory for AT+CIPSTART", socket->socket);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }

    sg_sprintf((char*) (commands[1].command),
               "AT+CIPSTART=%u,\"%s\",\"%s\",\"%u\"\r", socket->socket, socket->type, socket->host, socket->port);
    commands[1].delimiter = SOCKET_CONNECTION_DELIM;
    commands[1].logInfoCommand = "UDP-Socket-Connection-To-Server";
    commands[1].successStrings[0] = "CONNECT OK\r\n";
    commands[1].successStrings[1] = NULL;
    commands[1].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[2].command = NULL;
    rc = runBatchCommands("UDP-MODEM-SOCKET-SETUP", 0);

exit:
    if(commands[1].command)
        sg_free((char*) (commands[1].command));

    return rc;
}


static int setUpModemSocket(SG_Socket *socket)
{
    int rc = FAILURE;

    /*
     */
    commands[0].command = "AT+CIFSR\r";
    commands[0].delimiter = "\r\n";
    commands[0].logInfoCommand = "Get-IP-Address-Or-Connect-Will-Always-Fail";
    commands[0].successStrings[0] = "\r\n";
    commands[0].successStrings[1] = NULL;
    commands[0].commandInCaseNoSuccessStringPresent = NULL;

    if(sslEnabledAtSocketLayer == 1)
    {
        /*
         */
        commands[1].command = "AT+SSLOPT=1,1\r";
        commands[1].delimiter = OK_DELIMITER;
        commands[1].logInfoCommand = "Set-SSL-Options";
        commands[1].successStrings[0] = OK_DELIMITER;
        commands[1].successStrings[1] = NULL;
        commands[1].commandInCaseNoSuccessStringPresent = NULL;

        /*
         */
        commands[2].command = "AT+CIPSSL=1\r";
        commands[2].delimiter = OK_DELIMITER;
        commands[2].logInfoCommand = "Set-SSL";
        commands[2].successStrings[0] = OK_DELIMITER;
        commands[2].successStrings[1] = NULL;
        commands[2].commandInCaseNoSuccessStringPresent = NULL;

        /*
         */
        commands[3].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
        if(commands[3].command == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, MODEM_SOCKET "Could not allocate memory for AT+CIPSTART", socket->socket);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

        sg_sprintf((char*) (commands[3].command),
                   "AT+CIPSTART=%u,\"%s\",\"%s\",\"%u\"\r", socket->socket, socket->type, socket->host, socket->port);
        commands[3].delimiter = SOCKET_CONNECTION_DELIM;
        commands[3].logInfoCommand = "Socket-Connection-To-Server";
        commands[3].successStrings[0] = "CONNECT OK\r\n";
        commands[3].successStrings[1] = NULL;
        commands[3].commandInCaseNoSuccessStringPresent = NULL;


        /*
         */
        commands[4].command = NULL;
    }
    else
    {
        /*
         */
        commands[1].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
        if(commands[1].command == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, MODEM_SOCKET "Could not allocate memory for AT+CIPSTART", socket->socket);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

        sg_sprintf((char*) (commands[1].command),
                   "AT+CIPSTART=%u,\"%s\",\"%s\",\"%u\"\r", socket->socket, socket->type, socket->host, socket->port);
        commands[1].delimiter = SOCKET_CONNECTION_DELIM;
        commands[1].logInfoCommand = "Socket-Connection-To-Server";
        commands[1].successStrings[0] = "CONNECT OK\r\n";
        commands[1].successStrings[1] = NULL;
        commands[1].commandInCaseNoSuccessStringPresent = NULL;

        /*
         */
        commands[2].command = NULL;
    }

    rc = runBatchCommands("MODEM-SOCKET-SETUP", 0);

exit:
    if(sslEnabledAtSocketLayer == 1)
    {
        if(commands[3].command)
        {
            sg_free((char*) (commands[3].command));
        }
    }
    else
    {
        if(commands[1].command)
        {
            sg_free((char*) (commands[1].command));
        }
    }

    return rc;
}


/*
 * This method tries to establish the socket/socket to "socket->host" on "socket->port".
 *
 * If the connection is successful, then the following must be done by the device-implementation ::
 *                          socket->socketCorrupted = 0;
 *
 * Setting the above value will let InstaMsg know that the connection can be used fine for writing/reading.
 */
void simcom_5360_connect_underlying_socket_medium_try_once(SG_Socket* socket)
{
    int rc = FAILURE;

    sg_sprintf(LOG_GLOBAL_BUFFER, "(RE-)INITIALIZING MODEM");
    info_log(LOG_GLOBAL_BUFFER);

    rc = setUpModem(socket);
    if(rc != SUCCESS)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "COULD NOT INITIALIZE MODEM :(");
        info_log(LOG_GLOBAL_BUFFER);

        return;
    }

    sg_sprintf(LOG_GLOBAL_BUFFER, "MODEM INITIALIZATION DONE.");
    info_log(LOG_GLOBAL_BUFFER);

    bringUpWirelessOrReset(socket);

    socket->socket = nextConnectionNumber;
    nextConnectionNumber++;

    if(strcmp(socket->type, SOCKET_TCP) == 0)
    {
        rc = setUpModemSocket(socket);
    }
    else
    {
        rc = setUpModemSocketUDP(socket);
    }

    if(rc == SUCCESS)
    {
        /*
         * VERY IMPORTANT.. MUST BE DONE.
         */
        socket->socketCorrupted = 0;
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "MODEM-INITIALIZATION DONE... BUT SOCKET COULD NOT BE CONNECTED :(");
        info_log(LOG_GLOBAL_BUFFER);
    }
}


/*
 * This method reads "len" bytes from socket into "buffer".
 *
 * Exactly one of the cases must hold ::
 *
 * a)
 * "guaranteed" is 1.
 * So, this "read" must bahave as a blocking-read.
 *
 * Also, exactly "len" bytes are read successfully.
 * So, SUCCESS must be returned.
 *
 *                      OR
 *
 * b)
 * "guaranteed" is 1.
 * So, this "read" must bahave as a blocking-read.
 *
 * However, an error occurs while reading.
 * So, FAILURE must be returned immediately (i.e. no socket-reinstantiation must be done in this method).
 *
 *                      OR
 *
 * c)
 * "guaranteed" is 0.
 * So, this "read" must behave as a non-blocking read.
 *
 * Also, no bytes could be read in SOCKET_READ_TIMEOUT_SECS seconds (defined in "globals.h").
 * So, SOCKET_READ_TIMEOUT must be returned immediately.
 *
 *                      OR
 *
 * d)
 * "guaranteed" is 0.
 * So, this "read" must behave as a non-blocking read.
 *
 * Also, exactly "len" bytes are successfully read.
 * So, SUCCESS must be returned.
 *
 *                      OR
 *
 * e)
 * "guaranteed" is 0.
 * So, this "read" must behave as a non-blocking read.
 *
 * However, an error occurs while reading.
 * So, FAILURE must be returned immediately (i.e. no socket-reinstantiation must be done in this method).
 */
extern unsigned int bytes_received_over_wire;
int simcom_5360_socket_read(SG_Socket* socket, unsigned char* buffer, int len, unsigned char guaranteed)
{
    int bytesSoFar = 0;

    while(1)
    {
        memset(smallBuffer, 0, sizeof(smallBuffer));
        sg_sprintf(smallBuffer, "AT+CIPRXGET=2,%u,%d\r", socket->socket, len - bytesSoFar);

        memset(receiveBuffer, 0, sizeof(receiveBuffer));

        bytesFromServerBuffer = buffer + bytesSoFar;
        bytesActuallyRead = 0;

        bytesStartReading = 1;
        run_simple_at_command_and_get_output(smallBuffer, strlen(smallBuffer), receiveBuffer, sizeof(receiveBuffer),
                                             BYTES_POLL_RESP_HEADER, 0, 0);


        while(bytesStartReading == 1)
        {
            startAndCountdownTimer(1, 0);
        }

        bytesSoFar = bytesSoFar + bytesActuallyRead;
        if(bytesActuallyRead == len)
        {
            bytes_received_over_wire = bytes_received_over_wire + len;

            /*
             * Cases a) and d).
             */
            return SUCCESS;
        }
        else
        {
            if(errorObtained == 1)
            {
                /*
                 * Cases b) and e).
                 */
                errorObtained = 0;
                return FAILURE;
            }

            if(guaranteed == 0)
            {
                /*
                 * Case c).
                 */
                return SOCKET_READ_TIMEOUT;
            }
            else
            {
                continue;
            }
        }
    }

    /*
     * Just to make compiler happy.
     */
    return FAILURE;
}


/*
 * This method writes first "len" bytes from "buffer" onto the socket.
 *
 * This is a blocking function. So, either of the following must hold true ::
 *
 * a)
 * All "len" bytes are written.
 * In this case, SUCCESS must be returned.
 *
 *                      OR
 * b)
 * An error occurred while writing.
 * In this case, FAILURE must be returned immediately (i.e. no socket-reinstantiation must be done in this method).
 */
extern unsigned int bytes_sent_over_wire;
int simcom_5360_socket_write(SG_Socket* socket, unsigned char* buffer, int len)
{
    memset(smallBuffer, 0, sizeof(smallBuffer));
    sg_sprintf(smallBuffer, "AT+CIPSEND=%u,%d\r", socket->socket, len);

    memset(receiveBuffer, 0, sizeof(receiveBuffer));
    run_simple_at_command_and_get_output(smallBuffer, strlen(smallBuffer), receiveBuffer, sizeof(receiveBuffer), "\n>", 0, 0);

    memset(receiveBuffer, 0, sizeof(receiveBuffer));
    run_simple_at_command_and_get_output((char*)buffer, len, receiveBuffer, sizeof(receiveBuffer), "SEND OK\r\n", 0, 0);

    if(errorObtained == 0)
    {
        bytes_sent_over_wire = bytes_sent_over_wire + len;

        /*
         * Case a).
         */
        return SUCCESS;
    }
    else
    {
        /*
         * Case b).
         */
        errorObtained = 0;
        return FAILURE;
    }
}


/*
 * This method does the cleaning up (for eg. closing a socket) when the socket is cleaned up.
 * But if it is ok to re-connect without releasing the underlying-system-resource, then this can be left empty.
 *
 * Note that this method MUST DO """ONLY""" per-socket level cleanup, NO GLOBAL-LEVEL CLEANING/REINIT MUST BE DONE.
 */
void simcom_5360_release_underlying_socket_medium_guaranteed(SG_Socket* socket)
{
    waitBeforeReboot();
    resetDevice();
}


#if (SSL_ENABLED == 1) || (SOCKET_SSL_ENABLED == 1)
/*
 * This method loads the client-certificate into buffer.
 */
void simcom_5360_load_client_certificate_into_buffer(char *cert_buffer, int maxLength)
{
}


/*
 * This method saves the client-certificate onto the device in a persistent manner.
 */
void simcom_5360_save_client_certificate_from_buffer(char *cert_buffer)
{
}


/*
 * This method loads the client-private-key into buffer.
 */
void simcom_5360_load_client_private_key_into_buffer(char *private_key_buffer, int maxLength)
{
}


/*
 * This method saves the client-private-key onto the device in a persistent manner.
 */
void simcom_5360_save_client_private_key_from_buffer(char *private_key_buffer)
{
}

#endif

#endif
