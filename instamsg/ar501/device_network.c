/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "./uart_utils.h"

#include "../driver/include/instamsg.h"
#include "../driver/include/sg_mem.h"
#include "../driver/include/sg_stdlib.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"


typedef struct NetworkInitCommands NetworkInitCommands;

static char result[MAX_BUFFER_SIZE];
static char tempSmsBuffer[170];
static char *readBuffer;

static char *responseBuffer;
static unsigned int bytesRequired;
static unsigned int actualBytesRead;

static const char *specialDelimiter;

static volatile char resultObtained;
static volatile char errorObtained;
static volatile char noCarrierObtained;
static unsigned int ind;
static volatile char interruptsToBeUsed;
static volatile char commandIssued;
static volatile char showCommandOutput;


#define SEND_COMMAND_BUFFER_SIZE 100
static char sendCommandBuffer[SEND_COMMAND_BUFFER_SIZE];

char *noCarrier = "NO CARRIER";
char *noDial = "NO DIAL";
char *busy = "BUSY";

#define MODEM_SLEEP_INTERVAL 3
#define LENGTH_OF_COMMAND 0

static int parseNumberFromEndOfString(char *pch, char limiter)
{
    char smallBufReversed[4] = {0};
    char smallBufCorrect[4] = {0};
    int i, j;

    i = 0;

    while((*pch) != limiter)
    {
        memcpy(smallBufReversed + i, pch, 1);
        pch--;
        i++;
    }

    for(j = 0; j < i; j++)
    {
        memcpy(smallBufCorrect + j, smallBufReversed + i - 1 - j, 1);
    }

    return sg_atoi(smallBufCorrect);
}


void UART1Handler(void)
{
    unsigned long interrupts;
    interrupts  = UARTIntStatus(UART1_BASE, true);
    UARTIntClear(UART1_BASE, interrupts);

    /*
     * If we are in sync-mode, return immediately.
     */
    if((interruptsToBeUsed == 0) && (commandIssued == 1))
    {
        return;
    }

    while(1)
    {
        if(1)
        {
            if(1)
            {
                if(1)
                {
                    while(ROM_UARTCharsAvail(UART1_BASE))
                    {
                        readBuffer[ind++] = UARTCharGetNonBlocking(UART1_BASE);
                    }

                    readBuffer[ind] = 0;

                    if(specialDelimiter == NULL)
                    {
                        if(sg_mem_strstr(readBuffer, "\r\nOK\r\n", ind - 1) != NULL)
                        {
                            resultObtained = 1;
                        }
                        else if(sg_mem_strstr(readBuffer, "ERROR\r\n", ind - 1) != NULL)
                        {
                            resultObtained = 1;
                            errorObtained = 1;
                        }
                        else if(sg_mem_strstr(readBuffer, noCarrier, ind - 1) != NULL)
                        {
                            resultObtained = 1;
                            noCarrierObtained = 1;
                        }
                    }
                    else
                    {
                        if(sg_mem_strstr(readBuffer, specialDelimiter, ind - 1) != NULL)
                        {
                            resultObtained = 1;
                        }
                    }
                }
            }
        }

        break;
    }
}


void UART1Handler_Sync(void)
{
    char *ok = "OK";
    char *error = "ERROR";


    while(1)
    {
        if(1)
        {
            if(1)
            {
                char *triggerStringForActualData = "#SRECV";
                unsigned int newLineStart = 0;
                unsigned int actualDataStart = 0;
                unsigned int actualBytesAvailableInCurrentCycle = 0;
#if ENABLE_DEBUG_PROCESSING
                unsigned char commandEchoedFromGPSModule = 0;
#endif

                while(1)
                {
                    /*
                     * Extract "fro" from the total blocking/polling output of type
                     *
                     *
                     SRING: 1
                     AT#SRECV=1,3

                     #SRECV: 1,3
                     fro

                     OK

                     SRING: 1

                     NO CARRIER

                     */
                    readBuffer[ind++] = UARTCharGet(UART1_BASE);
                    if(readBuffer[ind - 1] == '\n')   /* We reached an end of line */
                    {
                        /*
                         * See if we got the trigger.
                         */
                        if(memcmp(readBuffer + newLineStart, triggerStringForActualData, strlen(triggerStringForActualData)) == 0)
                        {
                            actualDataStart = ind;
                            actualBytesAvailableInCurrentCycle = parseNumberFromEndOfString(readBuffer + ind - 3, ',');
                        }

                        /*
                         * See if we got any of the terminators.
                         */
                        else if(memcmp(readBuffer + newLineStart, ok, strlen(ok)) == 0)
                        {
                            break;
                        }
                        else if(memcmp(readBuffer + newLineStart, error, strlen(error)) == 0)
                        {
                            errorObtained = 1;
                            break;
                        }
                        else if((memcmp(readBuffer + newLineStart, noCarrier, strlen(noCarrier)) == 0) ||
                                (memcmp(readBuffer + newLineStart, noDial, strlen(noDial)) == 0) ||
                                (memcmp(readBuffer + newLineStart, busy, strlen(busy)) == 0))
                        {
                            noCarrierObtained = 1;
                            break;
                        }
#if ENABLE_DEBUG_PROCESSING
                        else if(memcmp(readBuffer + newLineStart, sendCommandBuffer, strlen(sendCommandBuffer)) == 0)
                        {
                            /*
                             * Bug in AR501's Micro-controller <==> Telit modules.
                             *
                             * Sometimes (indeterministically), we keep on getting the command-fired-to-Telit as response from Telit,
                             * and it enters into an infinite loop.
                             *
                             * See HANDLE_INIFINITE_RESPONSE_FROM_TELIT for the solution.
                             */
                            if(commandEchoedFromGPSModule == 1)
                            {
                                info_log("Entered into infinite loop :(");
                            }
                            commandEchoedFromGPSModule = 1;
                        }
#endif

                        /*
                         * Set the new-line-tracker.
                         */
                        newLineStart = ind;
                    }
                }

                /*
                 * If actualDataStart == 0, then we did not encounter the trigger "\r\n#SRECV" in the output
                 * (probably because of a "ERROR" or "NO CARRIER").
                 */
                if(actualDataStart != 0)
                {
                    memcpy(responseBuffer, readBuffer + actualDataStart, actualBytesAvailableInCurrentCycle);
                    actualBytesRead = actualBytesRead + actualBytesAvailableInCurrentCycle;
                }
            }
        }

        break;
    }

    readBuffer[ind] = 0;
}


struct NetworkInitCommands
{
    /*
     * The command to run.
     */
    char *command;

    /*
     * For logging purposes.
     */
    const char *logInfoCommand;

    /*
     * Assuming that there can be a maximum of 5 such strings.
     */
    char *successStrings[5];

    /*
     * If the command-output does not contain any of the expected-strings, this command
     * needs to be run for rectification.
     *
     * Ultimately, after running this command (1 or more times), the output of "command"
     * must contain one of the expected-strings.
     */
    char *commandInCaseNoSuccessStringPresent;
};
NetworkInitCommands commands[8];

#define MODEM           "[MODEM] "
#define COMMAND         "[COMMAND %u] "
#define MODEM_COMMAND   "[MODEM_INIT_COMMAND %u] "
#define MODEM_SOCKET    "[MODEM_SOCKET_COMMAND %u] "


static void SEND_CMD_AND_READ_RESPONSE_ON_UART1(const char *command, int len, char *desiredOutputBuffer, const char *delimiter)
{
    int lengthToSend = 0;

    if(len == LENGTH_OF_COMMAND)
    {
        lengthToSend = strlen(command);
    }
    else
    {
        lengthToSend = len;
    }

    /*
     * Reset the trackers for the command-status.
     */
    resultObtained = 0;
    errorObtained = 0;
    noCarrierObtained = 0;
    ind = 0;


    /*
     * We need to disable interrupt, during "read" operation.
     */
    if(desiredOutputBuffer != NULL)
    {
        interruptsToBeUsed = 0;
        responseBuffer = desiredOutputBuffer;
    }
    else
    {
        interruptsToBeUsed = 1;
        responseBuffer = NULL;
    }

    readBuffer = result;
    readBuffer[0] = 0;

    if(delimiter != NULL)
    {
        specialDelimiter = delimiter;
    }
    else
    {
        specialDelimiter = NULL;
    }

    commandIssued = 1;
    UARTSend(UART1_BASE, (unsigned char*)command, lengthToSend);
    if(responseBuffer == NULL)
    {
        while(resultObtained == 0)
        {
        }
    }
    else
    {
        UART1Handler_Sync();
    }

    if(showCommandOutput == 1)
    {
        info_log("Command = [%s], Output = [%s]", command, readBuffer);
    }
}


static int runBatchCommands(const char *batchName, unsigned char giveModemSleep)
{
    int i, j, passed, failed;

start_commands:

    if(giveModemSleep == 1)
    {
        info_log("Giving modem [%u] seconds, before checking all the settings and statuses", MODEM_SLEEP_INTERVAL);
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
            info_log("\n\n[%s] TOTAL COMMANDS: [%u], PASSED: [%u], FAILED: [%u]\n\n", batchName, i, passed, failed);
            break;
        }

        info_log("\n\n");
        info_log(COMMAND "Running [%s] for \"%s\"", i + 1, commands[i].command, commands[i].logInfoCommand);

        SEND_CMD_AND_READ_RESPONSE_ON_UART1(commands[i].command, LENGTH_OF_COMMAND, NULL, NULL);

        while(1)
        {
            j = 0;

            while(1)
            {

                if(commands[i].successStrings[j] == NULL)
                {
                    if(commands[i].commandInCaseNoSuccessStringPresent != NULL)
                    {
                        info_log(COMMAND "Initial Check for \"%s\" Failed.. trying to rectify with [%s]",
                                 i + 1, commands[i].logInfoCommand, commands[i].commandInCaseNoSuccessStringPresent);

                        SEND_CMD_AND_READ_RESPONSE_ON_UART1(commands[i].commandInCaseNoSuccessStringPresent, LENGTH_OF_COMMAND,
                                                            NULL, NULL);
                        goto start_commands;
                    }
                    else
                    {
                        info_log(COMMAND "\"%s\" Failed :(", i + 1, commands[i].logInfoCommand);

                        failed++;
                        break;
                    }
                }


                {
                    char *saveptr, *token, *temp;

                    RESET_GLOBAL_BUFFER;
                    strcpy((char*)GLOBAL_BUFFER, commands[i].successStrings[j]);

                    temp = (char*)GLOBAL_BUFFER;
                    while(1)
                    {
                        token = strtok_r(temp, "&", &saveptr);
                        if(token == NULL)
                        {
                            info_log(COMMAND "\"%s\" Passed", i + 1, commands[i].logInfoCommand);
                            passed++;

                            goto continue_with_next_command;
                        }

                        if(strstr(readBuffer, token) != NULL)
                        {
                            info_log(COMMAND "Found [%s] in output", i + 1, token);
                        }
                        else
                        {
                            goto continue_with_next_success_string;
                        }

                        /* For next call to strtok_r */
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


static int setUpModem(Network *network)
{
    int rc = FAILURE;
    showCommandOutput = 1;

    /*
     * Enable UART1.
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    /*
     * Enable RX/TX.
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinConfigure(GPIO_PC4_U1RX);
    GPIOPinConfigure(GPIO_PC5_U1TX);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    /*
     * Enable RTS/CTS.
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinConfigure(GPIO_PF0_U1RTS);
    GPIOPinConfigure(GPIO_PF1_U1CTS);
    GPIOPinTypeUART(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /*
     * Enable the UART-clocking
     */
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200,
                       (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_NONE));

   /*
    * Re-initializing Port-F :(
    */
    //RTS and CTS in IO mode
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);   //RTS output
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);          //CTS input
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);


    /*
     * D7 is used for RI detection of GPRS Modem
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    HWREG(GPIO_PORTD_BASE + GPIO_O_IM) &= ~(0xFF); /* Substitute of "GPIOPinIntDisable(GPIO_PORTD_BASE, 0xFF);"  */

    /* This is to enable GPIO capability on Pin PD7... Work around for NMI signal on the pin */
    HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY; /* Substitute of "HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD; */
    HWREG(GPIO_PORTD_BASE + GPIO_O_CR) = 0xff;
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_7);
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);


    /*
     * DCD Pin Monitor
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);


    /*
     * GPRS Reset IO
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);


    /*
     * GPRS Power Enable.
     */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);


    /*
     * Give time to Power-On to propel through ...
     */
    startAndCountdownTimer(2, 0);


    /*
     * VERY IMPORTANT: Reset GPS-module, to clear any left-over bytes in the buffers.
     *
     *                 Surprisingly, even power-off does not automatically does this.
     *                 This has to be done via hardware-pin.
     */
    {
        static unsigned char gpsResetDone = 0;

        if(gpsResetDone == 0)
        {
            info_log("Resetting GPS-module.");
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, 1);
            startAndCountdownTimer(2, 0);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, 0);
            info_log("Resetting GPS-module Done.\n\n");

            gpsResetDone = 1;
        }
    }


    ROM_UARTEnable(UART1_BASE);

    /*
     * Enable interrupts.
     */
    ROM_IntEnable(22); // Value of INT_UART1 from "inc/tm4c1230d5pm.h"
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT | UART_INT_CTS);



    /*
     * Prepare-init-commands.
     */
    commands[0].command = "AT#SIMDET?\r\n";
    commands[0].logInfoCommand = "SIM-Detect";
    commands[0].successStrings[0] = "1,0";
    commands[0].successStrings[1] = NULL;
    commands[0].commandInCaseNoSuccessStringPresent = "AT#SIMDET=1\r\n";


    /*
     */
    commands[1].command = "AT+CPIN?\r\n";
    commands[1].logInfoCommand = "SIM-PIN-Ready";
    commands[1].successStrings[0] = "READY";
    commands[1].successStrings[1] = NULL;
    commands[1].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[2].command = "AT+CREG?\r\n";
    commands[2].logInfoCommand = "SIM-Registered-To-Network";
    commands[2].successStrings[0] = "0,1";
    commands[2].successStrings[1] = "0,5";
    commands[2].successStrings[2] = NULL;
    commands[2].commandInCaseNoSuccessStringPresent = NULL;

    /*
     */
    commands[3].command = NULL;
    rc = runBatchCommands("MODEM-SETUP", 1);

    return rc;
}


static int setUpModemSocket(Network *network)
{
    int rc = FAILURE;


    /*
     */
    commands[0].command = "AT+CGDCONT?;#USERID?\r\n";
    commands[0].logInfoCommand = "GPRS-Context-Correctness";
    commands[0].successStrings[0] = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[0].successStrings[0] == NULL)
    {
        error_log("Could not allocate memory for successStrings[0] while testing for GPRS-context");
        goto exit;
    }
    else
    {
        sg_sprintf(commands[0].successStrings[0],"1,\"IP\",\"%s\"&#USERID: \"%s\"", network->gsmApn, network->gsmUser);
    }
    commands[0].successStrings[1] = NULL;
    commands[0].commandInCaseNoSuccessStringPresent = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[0].commandInCaseNoSuccessStringPresent == NULL)
    {
        error_log("Could not allocate memory for commandInCaseNoSuccessStringPresent while testing for GPRS-context");
        goto exit;
    }
    else
    {
        sg_sprintf(commands[0].commandInCaseNoSuccessStringPresent, "AT+CGDCONT=1,\"IP\",\"%s\";#USERID=\"%s\";#PASSW=\"%s\"\r\n",
                   network->gsmApn, network->gsmUser, network->gsmPass);
    }


    /*
     */
    commands[1].command = "AT+CGATT?\r\n";
    commands[1].logInfoCommand = "GPRS-Attachment-State";
    commands[1].successStrings[0] = "+CGATT: 1";
    commands[1].successStrings[1] = NULL;
    commands[1].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[2].command = "AT#SGACTCFG=1,15,180\r\n";
    commands[2].logInfoCommand = "Set-GPRS-Context-Config";
    commands[2].successStrings[0] = "\r\nOK\r\n";
    commands[2].successStrings[1] = NULL;
    commands[2].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[3].command = "AT#SGACT?\r\n";
    commands[3].logInfoCommand = "Actiavte-GPRS-PDP-Context-If-Not-Already";
    commands[3].successStrings[0] = "#SGACT: 1,1";
    commands[3].successStrings[1] = NULL;
    commands[3].commandInCaseNoSuccessStringPresent = "AT#SGACT=1,1\r\n";


    /*
     */
    commands[4].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[4].command == NULL)
    {
        error_log(MODEM_SOCKET "Could not allocate memory for AT#SCFG", network->socket);
        goto exit;
    }
    sg_sprintf(commands[4].command, "AT#SCFG=%u,1,512,0,600,50\r\n", network->socket);
    commands[4].logInfoCommand = "Socket-Configuration";
    commands[4].successStrings[0] = "\r\nOK\r\n";
    commands[4].successStrings[1] = NULL;
    commands[4].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[5].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[5].command == NULL)
    {
        error_log(MODEM_SOCKET "Could not allocate memory for AT#SCFGEXT", network->socket);
        goto exit;
    }
    sg_sprintf(commands[5].command, "AT#SCFGEXT= %u,0,0,0,0\r\n", network->socket);
    commands[5].logInfoCommand = "Extended-Socket-Configuration";
    commands[5].successStrings[0] = "\r\nOK\r\n";
    commands[5].successStrings[1] = NULL;
    commands[5].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[6].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[6].command == NULL)
    {
        error_log(MODEM_SOCKET "Could not allocate memory for AT#SD", network->socket);
        goto exit;
    }
    sg_sprintf(commands[6].command, "AT#SD=%u,0,%u,\"%s\",0,0,1\r\n", network->socket, INSTAMSG_PORT, INSTAMSG_HOST);
    commands[6].logInfoCommand = "Socket-Connection-To-Server";
    commands[6].successStrings[0] = "\r\nOK\r\n";
    commands[6].successStrings[1] = NULL;
    commands[6].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[7].command = NULL;
    rc = runBatchCommands("MODEM-SOCKET-SETUP", 0);

exit:
    if(commands[0].successStrings[0])
        sg_free(commands[0].successStrings[0]);

    if(commands[0].commandInCaseNoSuccessStringPresent)
        sg_free(commands[0].commandInCaseNoSuccessStringPresent);

    if(commands[4].command)
        sg_free(commands[4].command);

    if(commands[5].command)
        sg_free(commands[5].command);

    if(commands[6].command)
        sg_free(commands[6].command);


    return rc;
}


static void get_actual_command_output_for_command_results_with_ok_status(const char *command, const char *completeOutput, char *usefulOutput)
{
    int i, j;

    memcpy(usefulOutput,
           completeOutput + strlen(command),
           strlen(completeOutput) - strlen(strstr(completeOutput, "\r\nOK\r\n")) - strlen(command) + 1);

    /*
     * Remove trailing \r and \n (if any).
     */
    for(i = strlen(usefulOutput) - 1; i >= 0; i--)
    {
        if((usefulOutput[i] == '\r') || (usefulOutput[i] == '\n'))
        {
            usefulOutput[i] = 0;
        }
        else
        {
            break;
        }
    }

    /*
     * Remove leading \r and \n (if any).
     */
    for(i = 0; i < strlen(usefulOutput); i++)
    {
        if((usefulOutput[i] == '\r') || (usefulOutput[i] == '\n'))
        {
        }
        else
        {
            break;
        }
    }

    /*
     * Fill the empty-spaces that may have been created.
     */
    for(j = 0; j < strlen(usefulOutput); j++)
    {
        usefulOutput[j] = usefulOutput[j + i];
    }
}


#ifdef GSM_INTERFACE_ENABLED
/*
 * This method returns the *****LATEST****** sms, which contains the desired substring.
 *
 * Note that "{" are sometimes not processed correctly by some SIMs, so a prefix-match (which
 * otherwise is a stronger check) is not being done.
 *
 * Please note that this method is called by Instamsg-application, *****BEFORE***** calling
 * "connect_underlying_network_medium_try_once".
 */
void get_latest_sms_containing_substring(Network *network, char *buffer, const char *substr)
{
    int smsIndex = 1;
    /*
     * The modem must be set-up properly (SIM in place, etc.) for us to retrieve the sms.
     */
    while(setUpModem(network) == FAILURE)
    {
    }

    /*
     * Enable retrieving of SMS.
     */
    while(1)
    {
        SEND_CMD_AND_READ_RESPONSE_ON_UART1("AT+CMGF=1\r\n", LENGTH_OF_COMMAND, NULL, NULL);
        if(errorObtained == 0)
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

        memset(sendCommandBuffer, 0, SEND_COMMAND_BUFFER_SIZE);
        sg_sprintf(sendCommandBuffer, "AT+CMGR=%u\r\n", smsIndex);

        info_log("\n\nScanning SMS [%u] for Provisioning-Params", smsIndex);
        SEND_CMD_AND_READ_RESPONSE_ON_UART1(sendCommandBuffer, LENGTH_OF_COMMAND, NULL, NULL);

        if(errorObtained == 1)
        {
            /*
             * We stop scanning further SMSes.. if an ERROR-identifier was received as the command-output...
             */
            break;
        }
        else
        {
            /*
             * or the SMS-payload is empty.
             */
            unsigned char newLineStart = 0;
            int i;
            char *metadataPrefix = "+CMGR:";

            for(i = 0; i < ind; i++)
            {
                if(readBuffer[i] == '\n')
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
                            if(strncmp(readBuffer + newLineStart, metadataPrefix, strlen(metadataPrefix)) == 0)
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
                        tempBuffer[bufferIndex] = readBuffer[i];
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
        info_log("SMS-Payload = [%s]", tempBuffer);


        /*
         * If the substring is present, we copy from temporary-buffer to actual buffer.
         */
        if(strstr(tempBuffer, substr) != NULL)
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

    info_log("\n\n\n\nFinished scanning SMSes..");
    info_log("Provisioning-Info SMS extracted = [%s]", buffer);
}
#endif


/*
 * This method returns the univerally-unique-identifier for this device.
 */
void get_device_uuid(char *buffer)
{
    const char *command = "AT+CGSN\r\n";
    const char *constantPrefix = "ATOLL:IMEI:";

    do
    {
        SEND_CMD_AND_READ_RESPONSE_ON_UART1(command, LENGTH_OF_COMMAND, NULL, NULL);
    } while(errorObtained == 1);

    strcpy(buffer, constantPrefix);
    get_actual_command_output_for_command_results_with_ok_status(command, readBuffer, buffer + strlen(constantPrefix));

    info_log("\n\nIMEI received as UUID = [%s]\n\n", buffer);
}


/*
 * This method tries to establish the network/socket to "network->host" on "network->port".
 *
 * If the connection is successful, then the following must be done by the device-implementation ::
 *                          network->socketCorrupted = 0;
 *
 * Setting the above value will let InstaMsg know that the connection can be used fine for writing/reading.
 */
void connect_underlying_network_medium_try_once(Network* network)
{
    int rc = FAILURE;

    info_log("(RE-)INITIALIZING MODEM");

    rc = setUpModem(network);
    if(rc != SUCCESS)
    {
        info_log("COULD NOT INITIALIZE MODEM :(");
        return;
    }


    info_log("MODEM INITIALIZATION DONE.");



    /*
     * Next, we setup a socket, which will then be used for the usual read/write.
     */

    /*
     * 1. Get a free-socket
     */
    {
        char *pch;
        SEND_CMD_AND_READ_RESPONSE_ON_UART1("AT#SS\r\n", LENGTH_OF_COMMAND, NULL, NULL);

        /*
         * Search for a row, ending with ",0".
         * If such a row is found, it means there is a free-socket, waiting to be used !!
         */
        pch = strstr(result, ",0\r\n");
        if(pch != NULL)
        {
            /*
             * Get the connection/socket-id available.
             */
            network->socket = parseNumberFromEndOfString(pch - 1, ':');
            info_log(MODEM "Socket-Id obtained = [%u]", network->socket);
        }
    }


    /*
     * 2. Configure and Connect the socket.
     */
    rc = setUpModemSocket(network);
    if(rc == SUCCESS)
    {
        /*
         * VERY IMPORTANT.. MUST BE DONE.
         */
        network->socketCorrupted = 0;

        showCommandOutput = 0;
    }
    else
    {
        info_log("MODEM-INITIALIZATION DONE... BUT SOCKET COULD NOT BE CONNECTED :(");
    }
}

/*
 * This method reads "len" bytes from network into "buffer".
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
 * Also, no bytes could be read in NETWORK_READ_TIMEOUT_SECS seconds (defined in "globals.h").
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
int network_read(Network* network, unsigned char* buffer, int len, unsigned char guaranteed)
{
    /*
     * To resolve the issue mentioned in the HANDLE_INIFINITE_RESPONSE_FROM_TELIT marker, we MUST MUST MUST MUST
     * put a delay between any two "AT#SRECV" commands.
     */
    startAndCountdownTimer(NETWORK_READ_TIMEOUT_SECS, 0);

    bytesRequired = len;
    actualBytesRead = 0;

    while(len > actualBytesRead)
    {
        memset(sendCommandBuffer, 0, SEND_COMMAND_BUFFER_SIZE);
        sg_sprintf(sendCommandBuffer, "AT#SRECV=%u,%d\r\n", network->socket, len - actualBytesRead);

        SEND_CMD_AND_READ_RESPONSE_ON_UART1(sendCommandBuffer, LENGTH_OF_COMMAND, (char*)(buffer + actualBytesRead), NULL);


        /*
         * For "read", ERROR in AT#SRECV denotes no data in buffer.
         * So, this is kinda pseudo-timeout.
         */
        if(errorObtained == 1)
        {
            if(guaranteed == 1)
            {
                /*
                 * We don't care... we will wait till the data comes in ...
                 */
                continue;
            }
            else
            {
                /*
                 * Due to issue-and-its-corresponding-fix for HANDLE_INIFINITE_RESPONSE_FROM_TELIT marker, we do not need
                 * to do any extra timeout-processing here.
                 *
                 * So, if we still did not the response, we have genuinely timed-out !!
                 */
                return SOCKET_READ_TIMEOUT; /* Case c) */
            }
        }

        /*
         * For "read", NO CARRIED in AT#SRECV denotes that the GPRS-connection was lost.
         * Connection is broken.. so as per the API-requirement, return FAILURE.
         */
        if(noCarrierObtained)
        {
            return FAILURE; /* Case b) and e)  */
        }
    }

    return SUCCESS; /* Case a) and d) */
}


/*
 * This method writes first "len" bytes from "buffer" onto the network.
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
int network_write(Network* network, unsigned char* buffer, int len)
{
    memset(sendCommandBuffer, 0, SEND_COMMAND_BUFFER_SIZE);
    sg_sprintf(sendCommandBuffer, "AT#SSENDEXT=%u,%d\r\n", network->socket, len);

    SEND_CMD_AND_READ_RESPONSE_ON_UART1(sendCommandBuffer, LENGTH_OF_COMMAND, NULL, "\r\n>");
    SEND_CMD_AND_READ_RESPONSE_ON_UART1((char*)buffer, len, NULL, NULL);

    if((errorObtained == 0) && (noCarrierObtained == 0))
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}


/*
 * This method does the cleaning up (for eg. closing a socket) when the network is cleaned up.
 * But if it is ok to re-connect without releasing the underlying-system-resource, then this can be left empty.
 *
 * Note that this method MUST DO """ONLY""" per-socket level cleanup, NO GLOBAL-LEVEL CLEANING/REINIT MUST BE DONE.
 */
void release_underlying_network_medium_guaranteed(Network* network)
{
}

