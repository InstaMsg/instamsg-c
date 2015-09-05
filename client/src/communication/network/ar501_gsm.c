/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "instamsg_vendor.h"
#include "uart_utils.h"

#include "../../common/include/instamsg.h"

#include <string.h>

typedef struct NetworkInitCommands NetworkInitCommands;

static char result[MAX_BUFFER_SIZE];
static unsigned int ind;
static volatile char resultObtained;

#define MODEM_SLEEP_INTERVAL 30

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
                result[ind++] = UARTCharGetNonBlocking(UART1_BASE);
            }
        }

        break;
    }

    result[ind] = 0;

    /*
     * Now check if any of the terminator-strings is present.
     */
    if((strstr(result, "\r\nOK\r\n") != NULL) || (strstr(result, "ERROR") != NULL))
    {
        resultObtained = 1;
    }
}


struct NetworkInitCommands
{
    /*
     * The command to run.
     */
    const char *command;

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
     * Flag, which tells whether we should wait before re-running "command" after running "commandInCaseNoSuccessStringPresent".
     */
    unsigned char giveModemSomeSleep;
};
NetworkInitCommands commands[5];


#define MODEM "[MODEM] "
#define MODEM_COMMAND "[MODEM_INIT_COMMAND %u] "

static void runInitTests()
{
    int i, j, passed, failed;

    passed = 0;
    failed = 0;

    i = 0;
    while(1)
    {

start_command:
        if(commands[i].command == NULL)
        {
            info_log("\n\nTOTAL MODEM-INIT-COMMANDS: [%u], PASSED: [%u], FAILED: [%u]\n\n", i, passed, failed);
            break;
        }

        info_log("\n\n");
        info_log(MODEM_COMMAND "Running [%s] for \"%s\"", i, commands[i].command, commands[i].logInfoCommand);

        resultObtained = 0;
        ind = 0;

        UARTSend(UART1_BASE, (unsigned char*)commands[i].command, strlen(commands[i].command));
        while(resultObtained == 0)
        {
        }


        info_log(MODEM_COMMAND "COMMAND-OUTPUT = [%s]", i, result);

        while(1)
        {
            j = 0;

            while(1)
            {

                if(commands[i].successStrings[j] == NULL)
                {
                    if(commands[i].commandInCaseNoSuccessStringPresent != NULL)
                    {
                        info_log(MODEM_COMMAND "Initial Check for \"%s\" Failed.. trying to rectify with [%s]",
                                               i, commands[i].logInfoCommand, commands[i].commandInCaseNoSuccessStringPresent);

                        resultObtained = 0;
                        ind = 0;

                        UARTSend(UART1_BASE, (unsigned char*)commands[i].commandInCaseNoSuccessStringPresent,
                                 strlen(commands[i].commandInCaseNoSuccessStringPresent));
                        while(resultObtained == 0)
                        {
                        }

                        if(commands[i].giveModemSomeSleep == 1)
                        {
                            info_log(MODEM_COMMAND "Giving Modem [%u] seconds, before re-enquiring status", i, MODEM_SLEEP_INTERVAL);
                            startAndCountdownTimer(MODEM_SLEEP_INTERVAL, 1);
                        }

                        goto start_command;
                    }
                    else
                    {
                        info_log(MODEM_COMMAND "\"%s\" Failed :(", i, commands[i].logInfoCommand);

                        failed++;
                        break;
                    }
                }


                {
                    char *saveptr, *token, *temp;

                    RESET_GLOBAL_BUFFER;
                    strcpy((char*)GLOBAL_BUFFER, commands[i].successStrings[j]);

                    temp = GLOBAL_BUFFER;
                    while(1)
                    {
                        token = strtok_r(temp, "&", &saveptr);
                        if(token == NULL)
                        {
                            info_log(MODEM_COMMAND "\"%s\" Passed", i, commands[i].logInfoCommand);
                            passed++;

                            goto continue_with_next_command;
                        }

                        if(strstr(result, token) != NULL)
                        {
                            info_log(MODEM_COMMAND "Found [%s] in output", i, token);
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

    while(1)
    {
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
#if 1
    const char *apn = "ajay_apn";
    const char *userid = "ajay_userid";
    const char *passw = "ajay_passw";
#endif
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


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);                // DCD pin Monitor
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);                //GPRS Reset IO
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);                //GPRS Power Enable
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);
    GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);

    ROM_UARTEnable(UART1_BASE);

    /*
     * Give the GSM-Module enough time to bootstrap with the GSM-network.
     */
    info_log("Waiting for 30 seconds to let the GSM-module bootstrap with GSM-Network");
    startAndCountdownTimer(MODEM_SLEEP_INTERVAL, 1);

    /*
     * Enable interrupts.
     */
    ROM_IntEnable(INT_UART1);
    UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT | UART_INT_CTS);



    /*
     * Prepare-init-commands.
     */
    commands[0].command = "AT#SIMDET?\r\n";
    commands[0].logInfoCommand = "SIM-Detect";

    commands[0].successStrings[0] = "1,0";
    commands[0].successStrings[1] = NULL;

    commands[0].commandInCaseNoSuccessStringPresent = "AT#SIMDET=1\r\n";
    commands[0].giveModemSomeSleep = 1;


    /*
     */
    commands[1].command = "AT+CPIN?\r\n";
    commands[1].logInfoCommand = "SIM-Ready";

    commands[1].successStrings[0] = "READY";
    commands[1].successStrings[1] = NULL;

    commands[1].commandInCaseNoSuccessStringPresent = NULL;
    commands[1].giveModemSomeSleep = 1;


    /*
     */
    commands[2].command = "AT+CREG?\r\n";
    commands[2].logInfoCommand = "SIM-Registered-To-Network";

    commands[2].successStrings[0] = "0,1";
    commands[2].successStrings[1] = "0,5";
    commands[2].successStrings[2] = NULL;

    commands[2].commandInCaseNoSuccessStringPresent = NULL;
    commands[2].giveModemSomeSleep = 1;


    /*
     */
    commands[3].command = "AT+CGDCONT?;#USERID?\r\n";
    commands[3].logInfoCommand = "GPRS-Context-Correctness";

    commands[3].successStrings[0] = "OK";
    commands[3].successStrings[1] = NULL;

    commands[3].commandInCaseNoSuccessStringPresent = NULL;
    commands[3].giveModemSomeSleep = 1;


    /*
     */
    commands[4].command = NULL;


    runInitTests();
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
    return FAILURE;
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
    return FAILURE;
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void init_network(Network *network, const char *hostName, unsigned int port)
{
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
