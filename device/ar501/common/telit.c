#include "./telit.h"
#include "./uart_utils.h"

#include "../driver/include/sg_stdlib.h"
#include "../driver/include/sg_mem.h"

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


static char *responseBuffer;
static const char *specialDelimiter;

char *noCarrier = "NO CARRIER";
char *noDial = "NO DIAL";
char *busy = "BUSY";

static volatile char commandIssued;
static volatile char resultObtained;
static volatile char interruptsToBeUsed;


int parseNumberFromEndOfString(char *pch, char limiter)
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


static void UART1Handler_Sync(void)
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


void SEND_CMD_AND_READ_RESPONSE_ON_UART1(const char *command, int len, char *desiredOutputBuffer, const char *delimiter)
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


void get_actual_command_output_for_command_results_with_ok_status(const char *command, const char *completeOutput, char *usefulOutput)
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



