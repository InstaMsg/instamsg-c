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
#include "../driver/include/config.h"
#include "../driver/include/json.h"


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

#define SIM_CHECKER_CMD				"AT+CPIN?\r"

#define SIM_SLOT_INDEX				"SIM_SLOT_INDEX"
static char simSlotBuffer[500];

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
		unsigned char c = returnSingleCharacter();
		if(c == '\n')
		{
			break;
		}
	}
}


static void readTillNewLineAndAppendToBuffer(unsigned char *buffer, int sizeSoFar)
{
	while(1)
	{
		unsigned char c = returnSingleCharacter();	
	
		*(buffer + sizeSoFar) = c;
		sizeSoFar++;
		
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


#define BYTES_POLL_RESP_HEADER      "+CIPRXGET: 2,"
#define CIPSEND_HEADER				"\n>"

static void check_if_output_desired_and_available()
{
    int lengthOfDelimiter = strlen((char*)response_delimiter);
    int bytesReadTillThisIteration = modemReceiveBytesSoFar;

    if(1)
    {
        if(readResponse == 1)
        {
            unsigned char okToCheckForDelimiter = 0;
            if(1)
            {
                if(modemReceiveBytesSoFar > 0)
                {
					if(strcmp((char*) response_delimiter, CIPSEND_HEADER) == 0)
					{
						if(
							(*(modemReceiveBuffer + modemReceiveBytesSoFar - 1) == '>') &&
							(*(modemReceiveBuffer + modemReceiveBytesSoFar - 2) == '\n')
						  )
						  {
							  okToCheckForDelimiter = 1;
						  }
					}
					else
					{						
						if(*(modemReceiveBuffer + modemReceiveBytesSoFar - 1) == '\n')
						{
							okToCheckForDelimiter = 1;
						}
					}
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
				else if(strcmp((char*) response_delimiter, BYTES_POLL_RESP_HEADER) == 0)
				{
					const char *errorId = "\r\nERROR\r\n";
					
					char *ptr = (char*)sg_memnmem(modemReceiveBuffer, errorId, modemReceiveBytesSoFar, strlen(errorId));
					if(ptr != NULL)
					{
                        /*
                         * Send the bytes so-far to response-buffer.
                         */
                        memcpy((void*)responseBuffer, modemReceiveBuffer, modemReceiveBytesSoFar);
                        reset_modem_receive_buffer();

						bytesActuallyRead = 0;
						bytesStartReading = 0;
						readResponse = 0;
					}
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


static void switch_sim_slot(unsigned char rebootImmediately)
{
#if SIM_SLOT_SWITCHING_ENABLED == 1
	if(1)
	{
		int rc = FAILURE;
		int index = 0;
								
		memset(simSlotBuffer, 0, sizeof(simSlotBuffer));
		rc = get_config_value_from_persistent_storage(SIM_SLOT_INDEX, simSlotBuffer, sizeof(simSlotBuffer));
								
		if(rc == SUCCESS)
		{
			char small[4] = {0};
			getJsonKeyValueIfPresent(simSlotBuffer, CONFIG_VALUE_KEY, small);
									
			index = sg_atoi(small);		
		}
								
							
		watchdog_active = 0;
						
		{
			char command[50] = {0};
			memset(simSlotBuffer, 0, sizeof(simSlotBuffer));
										
			sg_sprintf(command, "AT+CGSETV=44,%u,1\r", index ^ 1);
								
			sg_sprintf(LOG_GLOBAL_BUFFER, "Firing command [%s]", command);
			info_log(LOG_GLOBAL_BUFFER);
									
			run_simple_at_command_and_get_output(command, strlen(command), simSlotBuffer, sizeof(simSlotBuffer), OK_DELIMITER, 1, 1);
																			 
			/*
			Save the toggled index on config.
			*/
									
			memset(simSlotBuffer, 0, sizeof(simSlotBuffer));
									
			{
				char small[4] = {0};
				sg_sprintf(small, "%u", index ^ 1);
		
				generate_config_json(simSlotBuffer, SIM_SLOT_INDEX, CONFIG_INT, small, "");
				save_config_value_on_persistent_storage(SIM_SLOT_INDEX, simSlotBuffer, 0);
			}
		}

		startAndCountdownTimer(1, 1);
		
		if(rebootImmediately == 1)
		{
			resetDevice();
		}
	}
#endif
}


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

		{
			memset(withoutTerminatedCommand, 0, sizeof(withoutTerminatedCommand));
			memcpy(withoutTerminatedCommand, commands[i].command, strlen(commands[i].command) - 1);
			
			sg_sprintf(LOG_GLOBAL_BUFFER, COMMAND "Running [%s] for \"%s\"", i + 1, withoutTerminatedCommand, commands[i].logInfoCommand);
			info_log(LOG_GLOBAL_BUFFER);
		}

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
						
						if(strcmp(commands[i].command, SIM_CHECKER_CMD) == 0)
						{
							sg_sprintf(LOG_GLOBAL_BUFFER, "SIM-Check failed .. next slot will be tried next time ..");
							error_log(LOG_GLOBAL_BUFFER);
							
							switch_sim_slot(1);
						}

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
							
							if(strcmp(commands[i].command, SIM_CHECKER_CMD) == 0)
							{
								sg_sprintf(LOG_GLOBAL_BUFFER, "Reading two additional lines from modem ..");
								info_log(LOG_GLOBAL_BUFFER);
								
								read_till_newline();
								read_till_newline();
							}
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
static unsigned char minimalModemSetupDone;

static int setUpModemMinimal()
{
	int i = -1, rc = FAILURE;
	
	if(minimalModemSetupDone == 1)
	{
		sg_sprintf(LOG_GLOBAL_BUFFER, "\"setUpModemMinimal\" step already done successfully, not re-doing");
		info_log(LOG_GLOBAL_BUFFER);
		
		return SUCCESS;
	}
	
    /*
     * Prepare-init-commands.
     */

    /*
     */
	i++;
    commands[i].command = SIM_CHECKER_CMD;
    commands[i].delimiter = "+";
    commands[i].logInfoCommand = "SIM-PIN-Ready";
    commands[i].successStrings[0] = "READY";
    commands[i].successStrings[1] = NULL;
    commands[i].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
	i++;
    commands[i].command = "AT+CIPRXGET=1\r";
    commands[i].delimiter = OK_DELIMITER;
    commands[i].logInfoCommand = "Enable-Data-Sync-Read";
    commands[i].successStrings[0] = OK_DELIMITER;
    commands[i].successStrings[1] = NULL;
    commands[i].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
	i++;
    commands[i].command = "AT+CIPHEAD=0\r";
    commands[i].delimiter = OK_DELIMITER;
    commands[i].logInfoCommand = "Disable-IP-Header-Async-Notification";
    commands[i].successStrings[0] = OK_DELIMITER;
    commands[i].successStrings[1] = NULL;
    commands[i].commandInCaseNoSuccessStringPresent = NULL;
	
	
    /*
     */
	i++;
    commands[i].command = "AT+CIPSRIP=0\r";
    commands[i].delimiter = OK_DELIMITER;
    commands[i].logInfoCommand = "Disable-Data-Received-Prompt";
    commands[i].successStrings[0] = OK_DELIMITER;
    commands[i].successStrings[1] = NULL;
    commands[i].commandInCaseNoSuccessStringPresent = NULL;
	
	
#if ( (SEND_GPS_LOCATION == 1) || (GPS_TIME_SYNC_PRESENT == 1) )
	i++;
	commands[i].command = "AT+CGPS=1,1\r";
	commands[i].delimiter = OK_DELIMITER;
	commands[i].logInfoCommand = "Start-GPS";
	commands[i].successStrings[0] = OK_DELIMITER;
	commands[i].successStrings[1] = NULL;
	commands[i].commandInCaseNoSuccessStringPresent = NULL;
#endif

    /*
     */
	i++;
    commands[i].command = NULL;
	
	
    rc =  runBatchCommands("MODEM-SETUP-MINIMAL", 1);
	if(rc == SUCCESS)
	{
		minimalModemSetupDone = 1;
	}
	
	return rc;
}


static int setUpModem(SG_Socket *socket)
{
    int rc = FAILURE;
	int i = -1;
	int sockContIndex = -1;

    if(setUpModemDone == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "%s\"setUpModem\" step already done successfully, not re-doing", SOCKET);
        info_log(LOG_GLOBAL_BUFFER);

        return SUCCESS;
    }

    while(setUpModemMinimal(socket) != SUCCESS)
    {
    }

    /*
     */
	i++;
    commands[i].command = "AT+CREG?\r";
    commands[i].delimiter = OK_DELIMITER;
    commands[i].logInfoCommand = "SIM-Registered-To-Network";
    commands[i].successStrings[0] = "0,1";
    commands[i].successStrings[1] = "0,5";
    commands[i].successStrings[2] = NULL;
    commands[i].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
	i++;
    commands[i].command = "AT+CGATT?\r";
    commands[i].delimiter = OK_DELIMITER;
    commands[i].logInfoCommand = "GPRS-Attachment-State";
    commands[i].successStrings[0] = "+CGATT: 1";
    commands[i].successStrings[1] = NULL;
    commands[i].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
	i++;
	sockContIndex = i;
    commands[i].command = (char*) sg_malloc(MAX_BUFFER_SIZE);
	if(commands[i].command == NULL)
	{
		sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory for CGSOCKCONT setting");
		error_log(LOG_GLOBAL_BUFFER);

		goto exit;
	}	
	else
	{
		sg_sprintf((char*) (commands[i].command), "AT+CGSOCKCONT=1,\"IP\",\"%s\"\r", socket->gsmApn);
	}
    commands[i].delimiter = OK_DELIMITER;
    commands[i].logInfoCommand = "CGSOCKCONT-Setting";
    commands[i].successStrings[0] = OK_DELIMITER;
    commands[i].successStrings[1] = NULL;
    commands[i].commandInCaseNoSuccessStringPresent = NULL;
	
	
	/*
     */
	i++;
    commands[i].command = "AT+CSOCKSETPN=1\r";
    commands[i].delimiter = OK_DELIMITER;
    commands[i].logInfoCommand = "CGSOCKSETPN-Setting";
    commands[i].successStrings[0] = OK_DELIMITER;
    commands[i].successStrings[1] = NULL;
    commands[i].commandInCaseNoSuccessStringPresent = NULL;
	
	
	/*
     */
	i++;
    commands[i].command = "AT+NETOPEN\r";
    commands[i].delimiter = "\r\n+NETOPEN:";
    commands[i].logInfoCommand = "Open-Network";
    commands[i].successStrings[0] = "\r\n+NETOPEN: 0\r\n";
    commands[i].successStrings[1] = NULL;
    commands[i].commandInCaseNoSuccessStringPresent = NULL;
	
	
    /*
     */
	i++;
    commands[i].command = NULL;
    rc = runBatchCommands("MODEM-SETUP", 1);


exit:
    if(commands[sockContIndex].command)
        sg_free((char*) (commands[sockContIndex].command));


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
#define SMS_STORAGE_AREA_COMMAND		"AT+CPMS=\"SM\",\"SM\",\"SM\"\r"
#define SMS_MODE_SETTER_COMMAND         "AT+CMGF=1\r"

static char tempSmsBuffer[1000];

void simcom_5360_get_latest_sms_containing_substring(SG_Socket *socket, char *buffer, const char *substring)
{
    int smsIndex = 1;
	
	while(setUpModemMinimal() != SUCCESS)
	{
	}
	
    /*
     * Enable retrieving of SMS.
     */
    if(1)
    {
		memset(messageBuffer, 0, sizeof(messageBuffer));
		run_simple_at_command_and_get_output(SMS_STORAGE_AREA_COMMAND, strlen(SMS_STORAGE_AREA_COMMAND),
											 messageBuffer, sizeof(messageBuffer), OK_DELIMITER, 1, 0);	
											 			
		memset(messageBuffer, 0, sizeof(messageBuffer));
        run_simple_at_command_and_get_output(SMS_MODE_SETTER_COMMAND, strlen(SMS_MODE_SETTER_COMMAND),
                                             messageBuffer, sizeof(messageBuffer), OK_DELIMITER, 1, 0);
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

        run_simple_at_command_and_get_output(smallBuffer, strlen(smallBuffer), messageBuffer, sizeof(messageBuffer), "\r\n+C", 1, 0);
		if(strstr(messageBuffer, "ERROR: Invalid memory index") != NULL)
		{
			sg_sprintf(LOG_GLOBAL_BUFFER, "No more SMSes to read ... ");
			info_log(LOG_GLOBAL_BUFFER);
			
			break;
		}
		else
		{
			readTillNewLineAndAppendToBuffer((unsigned char*) messageBuffer, strlen(messageBuffer));
			readTillNewLineAndAppendToBuffer((unsigned char*) messageBuffer, strlen(messageBuffer));
			readTillNewLineAndAppendToBuffer((unsigned char*) messageBuffer, strlen(messageBuffer));
		}
		
		sg_sprintf(LOG_GLOBAL_BUFFER, "Complete response of AT+CMGR [%s]", messageBuffer);
		info_log(LOG_GLOBAL_BUFFER);

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


static unsigned int nextConnectionNumber;
static int setUpModemSocketUDP(SG_Socket *socket)
{
    int rc = FAILURE;

    if(0)
    {
    }
    else
    {
        /*
         */
        commands[0].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
        if(commands[0].command == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, MODEM_SOCKET "Could not allocate memory for AT+CIPOPEN", socket->socket);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

        sg_sprintf((char*) (commands[0].command),
                   "AT+CIPOPEN=%u,\"%s\",,,9000\r", socket->socket, socket->type);
        commands[0].delimiter = "+CIPOPEN: ";
        commands[0].logInfoCommand = "UDP-Socket-Connection-To-Server";
		commands[0].successStrings[0] = (char*)sg_malloc(MAX_BUFFER_SIZE);
        if(commands[0].successStrings[0] == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, MODEM_SOCKET "Could not allocate memory for CIPOPEN socket-check", socket->socket);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }
		
        sg_sprintf((char*) (commands[0].successStrings[0]), "+CIPOPEN: %u,0", socket->socket);
        commands[0].successStrings[1] = NULL;
        commands[0].commandInCaseNoSuccessStringPresent = NULL;

        /*
         */
        commands[1].command = NULL;
    }

    rc = runBatchCommands("UDP-MODEM-SOCKET-SETUP", 0);

exit:
    if(sslEnabledAtSocketLayer == 1)
    {
    }
    else
    {
        if(commands[0].successStrings[0])
        {
            sg_free((char*) (commands[0].successStrings[0]));
        }
		if(commands[0].command)
        {
            sg_free((char*) (commands[0].command));
        }
    }

    return rc;
}


static int setUpModemSocket(SG_Socket *socket)
{
    int rc = FAILURE;

    if(sslEnabledAtSocketLayer == 1)
    {
    }
    else
    {
        /*
         */
        commands[0].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
        if(commands[0].command == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, MODEM_SOCKET "Could not allocate memory for AT+CIPOPEN", socket->socket);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

        sg_sprintf((char*) (commands[0].command),
                   "AT+CIPOPEN=%u,\"%s\",\"%s\",%u\r", socket->socket, socket->type, socket->host, socket->port);
        commands[0].delimiter = "+CIPOPEN: ";
        commands[0].logInfoCommand = "Socket-Connection-To-Server";
		commands[0].successStrings[0] = (char*)sg_malloc(MAX_BUFFER_SIZE);
        if(commands[0].successStrings[0] == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, MODEM_SOCKET "Could not allocate memory for CIPOPEN socket-check", socket->socket);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }
		
        sg_sprintf((char*) (commands[0].successStrings[0]), "+CIPOPEN: %u,0", socket->socket);
        commands[0].successStrings[1] = NULL;
        commands[0].commandInCaseNoSuccessStringPresent = NULL;

        /*
         */
        commands[1].command = NULL;
    }

    rc = runBatchCommands("MODEM-SOCKET-SETUP", 0);

exit:
    if(sslEnabledAtSocketLayer == 1)
    {
    }
    else
    {
        if(commands[0].successStrings[0])
        {
            sg_free((char*) (commands[0].successStrings[0]));
        }
		if(commands[0].command)
        {
            sg_free((char*) (commands[0].command));
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
		
		switch_sim_slot(0);
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
        if(bytesSoFar == len)
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
    memset(receiveBuffer, 0, sizeof(receiveBuffer));

    if(strcmp(socket->type, SOCKET_TCP) == 0)
    {
        sg_sprintf(smallBuffer, "AT+CIPSEND=%u,%d\r", socket->socket, len);
    }
    else
    {
        sg_sprintf(smallBuffer, "AT+CIPSEND=%u,%d,\"%s\",%u\r", socket->socket, len, socket->host, socket->port);
    }

    run_simple_at_command_and_get_output(smallBuffer, strlen(smallBuffer), receiveBuffer, sizeof(receiveBuffer), CIPSEND_HEADER, 0, 0);

    memset(receiveBuffer, 0, sizeof(receiveBuffer));
    run_simple_at_command_and_get_output((char*)buffer, len, receiveBuffer, sizeof(receiveBuffer), "\r\n+CIPSEND: ", 0, 0);

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
