/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "instamsg_vendor.h"
#include "uart_utils.h"

#include "../../common/include/instamsg.h"
#include "../../common/include/sg_mem.h"
#include "../../common/include/sg_stdlib.h"

#include <string.h>

typedef struct NetworkInitCommands NetworkInitCommands;

static char result[MAX_BUFFER_SIZE];
static char *readBuffer;
static const char *specialDelimiter;

static unsigned int ind;
static volatile char resultObtained;
static unsigned char trackDebugResponse;

#define SEND_COMMAND_BUFFER_SIZE 100
static char sendCommandBuffer[SEND_COMMAND_BUFFER_SIZE];
    static char small[10];

#define MODEM_SLEEP_INTERVAL 30
#define LENGTH_OF_COMMAND 0

void UART1Handler(void)
{
    unsigned long interrupts;

    interrupts  = UARTIntStatus(UART1_BASE, true);
    UARTIntClear(UART1_BASE, interrupts);

    while(1)
    {
        if(1)
        {
            while(ROM_UARTCharsAvail(UART1_BASE))
            {
                readBuffer[ind++] = UARTCharGetNonBlocking(UART1_BASE);
            }
        }

        break;
    }

    readBuffer[ind] = 0;
    if(trackDebugResponse == 1)
    {
        info_log("*");
        info_log(readBuffer);
    }

    /*
     * Now check if any of the terminator-strings is present.
     */
    if(specialDelimiter == NULL)
    {
        if((strstr(readBuffer, "\r\nOK\r\n") != NULL) || (strstr(readBuffer, "ERROR") != NULL))
        {
            resultObtained = 1;
        }
    }
    else
    {
        if(strstr(readBuffer, specialDelimiter) != NULL)
        {
            resultObtained = 1;
        }
    }
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


static void SEND_CMD_AND_READ_RESPONSE_ON_UART1(const char *command, int len, char *buffer, unsigned char showCommandOutput,
                                                const char *delimiter)
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

    resultObtained = 0;
    ind = 0;

    readBuffer = buffer;
    if(delimiter != NULL)
    {
        specialDelimiter = delimiter;
    }
    else
    {
        specialDelimiter = NULL;
    }

    UARTSend(UART1_BASE, (unsigned char*)command, lengthToSend);
    while(resultObtained == 0)
    {
    }

    if(showCommandOutput == 1)
    {
        info_log("Command = [%s], Output = [%s]", command, result);
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

        SEND_CMD_AND_READ_RESPONSE_ON_UART1(commands[i].command, LENGTH_OF_COMMAND, result, 1, NULL);

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
                                                            result, 1, NULL);
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

/*
 * This method does the cleaning up (for eg. closing a socket) when the network is cleaned up.
 *
 * Note that this method MUST DO """ONLY""" per-socket level cleanup, NO GLOBAL-LEVEL CLEANING/REINIT MUST BE DONE.
 */
static void release_underlying_medium_guaranteed(Network* network)
{
}


static int setUpModem()
{
#if 1
    const char *apn = "www";
    const char *userid = "";
    const char *passw = "";
#endif
    int rc = FAILURE;

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
    commands[3].command = "AT+CGDCONT?;#USERID?\r\n";
    commands[3].logInfoCommand = "GPRS-Context-Correctness";
    commands[3].successStrings[0] = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[3].successStrings[0] == NULL)
    {
        error_log("Could not allocate memory for successStrings[0] while testing for GPRS-context");
        goto exit;
    }
    else
    {
        sg_sprintf(commands[3].successStrings[0],"1,\"IP\",\"%s\"&#USERID: \"%s\"", apn, userid);
    }
    commands[3].successStrings[1] = NULL;
    commands[3].commandInCaseNoSuccessStringPresent = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[3].commandInCaseNoSuccessStringPresent == NULL)
    {
        error_log("Could not allocate memory for commandInCaseNoSuccessStringPresent while testing for GPRS-context");
        goto exit;
    }
    else
    {
        sg_sprintf(commands[3].commandInCaseNoSuccessStringPresent, "AT+CGDCONT=1,\"IP\",\"%s\";#USERID=\"%s\";#PASSW=\"%s\"\r\n",
                   apn, userid, passw);
    }


    /*
     */
    commands[4].command = "AT+CGATT?\r\n";
    commands[4].logInfoCommand = "GPRS-Attachment-State";
    commands[4].successStrings[0] = "+CGATT: 1";
    commands[4].successStrings[1] = NULL;
    commands[4].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[5].command = "AT#SGACTCFG=1,15,180\r\n";
    commands[5].logInfoCommand = "Set-GPRS-Context-Config";
    commands[5].successStrings[0] = "\r\nOK\r\n";
    commands[5].successStrings[1] = NULL;
    commands[5].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[6].command = "AT#SGACT?\r\n";
    commands[6].logInfoCommand = "Actiavte-GPRS-PDP-Context-If-Not-Already";
    commands[6].successStrings[0] = "#SGACT: 1,1";
    commands[6].successStrings[1] = NULL;
    commands[6].commandInCaseNoSuccessStringPresent = "AT#SGACT=1,1\r\n";


    /*
     */
    commands[7].command = NULL;
    rc = runBatchCommands("MODEM-CONFIGURATION", 1);

exit:
    if(commands[3].successStrings[0])
        sg_free(commands[3].successStrings[0]);

    if(commands[3].commandInCaseNoSuccessStringPresent)
        sg_free(commands[3].commandInCaseNoSuccessStringPresent);


    return rc;
}


static int setUpModemSocket(int socketId)
{
    int rc = FAILURE;

    /*
     */
    commands[0].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[0].command == NULL)
    {
        error_log(MODEM_SOCKET "Could not allocate memory for AT#SCFG", socketId);
        goto exit;
    }
    sg_sprintf(commands[0].command, "AT#SCFG=%u,1,512,0,600,50\r\n", socketId);
    commands[0].logInfoCommand = "Socket-Configuration";
    commands[0].successStrings[0] = "\r\nOK\r\n";
    commands[0].successStrings[1] = NULL;
    commands[0].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[1].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[1].command == NULL)
    {
        error_log(MODEM_SOCKET "Could not allocate memory for AT#SCFGEXT", socketId);
        goto exit;
    }
    sg_sprintf(commands[1].command, "AT#SCFGEXT= %u,0,0,0,0\r\n", socketId);
    commands[1].logInfoCommand = "Extended-Socket-Configuration";
    commands[1].successStrings[0] = "\r\nOK\r\n";
    commands[1].successStrings[1] = NULL;
    commands[1].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[2].command = (char*)sg_malloc(MAX_BUFFER_SIZE);
    if(commands[2].command == NULL)
    {
        error_log(MODEM_SOCKET "Could not allocate memory for AT#SD", socketId);
        goto exit;
    }
    sg_sprintf(commands[2].command, "AT#SD=%u,0,32000,\"platform.instamsg.io\",0,0,1\r\n", socketId);
    commands[2].logInfoCommand = "Socket-Connection-To-Server";
    commands[2].successStrings[0] = "\r\nOK\r\n";
    commands[2].successStrings[1] = NULL;
    commands[2].commandInCaseNoSuccessStringPresent = NULL;


    /*
     */
    commands[3].command = NULL;
    rc = runBatchCommands("MODEM-SOCKET-CONFIGURATION", 0);

exit:
    if(commands[0].command)
        sg_free(commands[0].command);

    if(commands[1].command)
        sg_free(commands[1].command);

    if(commands[2].command)
        sg_free(commands[2].command);


    return rc;
}


/*
 * This method tries to establish the network/socket to the "hostName" and "port".
 *
 * If the connection is successful, then the following must be done by the device-implementation ::
 *                          network->socketCorrupted = 0;
 *
 * Setting the above value will let InstaMsg know that the connection can be used fine for writing/reading.
 */
static void connect_underlying_medium_try_once(Network* network, char *hostName, int port)
{
    int rc = FAILURE;

    do
    {
        do
        {
            info_log("(RE-)INITIALIZING MODEM");
            rc = setUpModem();
        } while(rc == FAILURE);

        info_log("MODEM INITIALIZATION DONE.");

        /*
        * Next, we setup a socket, which will then be used for the usual read/write.
        */

        /*
        * 1. Get a free-socket
        */
        {
            char *pch;

            SEND_CMD_AND_READ_RESPONSE_ON_UART1("AT#SS\r\n", LENGTH_OF_COMMAND, result, 1, NULL);

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
                char smallBufReversed[4] = {0};
                char smallBufCorrect[4] = {0};
                int i, j;

                pch--;
                i = 0;

                while((*pch) != ':')
                {
                    memcpy(smallBufReversed + i, pch, 1);
                    pch--;
                    i++;
                }

                for(j = 0; j < i; j++)
                {
                    memcpy(smallBufCorrect + j, smallBufReversed + i - 1 - j, 1);
                }

                network->socket = sg_atoi(smallBufCorrect);
                info_log(MODEM "Socket-Id obtained = [%u]", network->socket);
            }
        }

        rc = setUpModemSocket(network->socket);
    } while(rc == FAILURE);

    network->socketCorrupted = 0;
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
static int ar501_gsm_socket_read(Network* network, unsigned char* buffer, int len, unsigned char guaranteed)
{
    memset(sendCommandBuffer, 0, SEND_COMMAND_BUFFER_SIZE);
    sg_sprintf(sendCommandBuffer, "AT#SRECV=%u,%d\r\n", network->socket, len);

    trackDebugResponse = 1;
    SEND_CMD_AND_READ_RESPONSE_ON_UART1(sendCommandBuffer, LENGTH_OF_COMMAND, (char*)buffer, 1, NULL);

    return SUCCESS;

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
static int ar501_gsm_socket_write(Network* network, unsigned char* buffer, int len)
{
    memset(sendCommandBuffer, 0, SEND_COMMAND_BUFFER_SIZE);
    sg_sprintf(sendCommandBuffer, "AT#SSENDEXT=%u,%d\r\n", network->socket, len);
    SEND_CMD_AND_READ_RESPONSE_ON_UART1(sendCommandBuffer, LENGTH_OF_COMMAND, result, 1, "\r\n>");

    SEND_CMD_AND_READ_RESPONSE_ON_UART1((char*)buffer, len, result, 1, NULL);

    return SUCCESS;
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void init_network(Network *network, const char *hostName, unsigned int port)
{
    //strictly
    char *test = "this is awesome";

    /* Register read-callback. */
	network->read = ar501_gsm_socket_read;

    /* Register write-callback. */
	network->write = ar501_gsm_socket_write;

    /* Keep a copy of connection-parameters, for easy book-keeping. */
    memset(network->host, 0, MAX_BUFFER_SIZE);
    sg_sprintf(network->host, "%s", hostName);
    network->port = port;

    /* Connect the medium. */
    connect_underlying_medium_try_once(network, network->host, network->port);


    //strictly
    network->write(network, (unsigned char*)test, strlen(test));
    memset(small, 0, 10);
    network->read(network, (unsigned char*)small, 3, 1);
    memset(small, 0, 10);
    network->read(network, (unsigned char*)small, 2, 1);
    memset(small, 0, 10);
    network->read(network, (unsigned char*)small, 3, 1);
    while(1)
    {
    }
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void release_network(Network *network)
{
    release_underlying_medium_guaranteed(network);

    info_log("COMPLETE [TCP-SOCKET] STRUCTURE, INCLUDING THE UNDERLYING MEDIUM (SOCKET) CLEANED FOR HOST = [%s], PORT = [%d].",
             network->host, network->port);
}
