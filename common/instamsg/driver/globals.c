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



#include "./include/globals.h"
#include "./include/instamsg.h"
#include "./include/time.h"
#include "./include/watchdog.h"
#include "./include/misc.h"
#include "./include/json.h"
#include "./include/sg_stdlib.h"
#include "./include/config.h"
#include "./include/data_logger.h"
#include "./include/at.h"


#include <string.h>
#include <stdarg.h>


static void sg_varargs(char *out, const char *pcString, va_list vaArgP)
{
    unsigned long ulIdx, ulValue, ulPos, ulCount, ulBase, ulNeg, temp;
    char *pcStr, pcBuf[16], cFill;

    /*
     * Loop while there are more characters in the string.
     */
    while(*pcString)
    {
        /*
         * Find the first non-% character, or the end of the string.
         */
        for(ulIdx = 0; (pcString[ulIdx] != '%') && (pcString[ulIdx] != '\0');
            ulIdx++)
        {
        }

        /*
         * Write this portion of the string.
         */
        for(temp = 0; temp < ulIdx; temp++)
        {
            *out = pcString[temp];
            out++;
        }

        /*
         * Skip the portion of the string that was written.
         */
        pcString += ulIdx;

        /*
         * See if the next character is a %.
         */
        if(*pcString == '%')
        {
            /*
             * Skip the %.
             */
            pcString++;

            /*
             * Set the digit count to zero, and the fill character to space
             * (i.e. to the defaults).
             */
            ulCount = 0;
            cFill = ' ';

            /*
             * It may be necessary to get back here to process more characters.
             * Goto's aren't pretty, but effective.  I feel extremely dirty for
             * using not one but two of the beasts.
             */
again:

            /*
             * Determine how to handle the next character.
             */
            switch(*pcString++)
            {
                /*
                 * Handle the digit characters.
                 */
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    /*
                     * If this is a zero, and it is the first digit, then the
                     * fill character is a zero instead of a space.
                     */
                    if((pcString[-1] == '0') && (ulCount == 0))
                    {
                        cFill = '0';
                    }

                    /*
                     * Update the digit count.
                     */
                    ulCount *= 10;
                    ulCount += pcString[-1] - '0';

                    /*
                     * Get the next character.
                     */
                    goto again;
                }

                /*
                 * Handle the %c command.
                 */
                case 'c':
                {
                    /*
                     * Get the value from the varargs.
                     */
                    ulValue = va_arg(vaArgP, unsigned long);

                    /*
                     * Print out the character.
                     */
                    *out = (char)ulValue;
                    out++;

                    /*
                     * This command has been handled.
                     */
                    break;
                }

                /*
                 * Handle the %d and %i commands.
                 */
                case 'd':
                case 'i':
                {
                    /*
                     * Get the value from the varargs.
                     */
                    ulValue = va_arg(vaArgP, unsigned long);

                    /*
                     * Reset the buffer position.
                     */
                    ulPos = 0;

                    /*
                     * If the value is negative, make it positive and indicate
                     * that a minus sign is needed.
                     */
                    if((long)ulValue < 0)
                    {
                        /*
                         * Make the value positive.
                         */
                        ulValue = -(long)ulValue;

                        /*
                         * Indicate that the value is negative.
                         */
                        ulNeg = 1;
                    }
                    else
                    {
                        /*
                         * Indicate that the value is positive so that a minus
                         * sign isn't inserted.
                         */
                        ulNeg = 0;
                    }

                    /*
                     * Set the base to 10.
                     */
                    ulBase = 10;

                    /*
                     * Convert the value to ASCII.
                     */
                    goto convert;
                }

                /*
                 * Handle the %s command.
                 */
                case 's':
                {
                    /*
                     * Get the string pointer from the varargs.
                     */
                    pcStr = va_arg(vaArgP, char *);

                    /*
                     * Determine the length of the string.
                     */
                    for(ulIdx = 0; pcStr[ulIdx] != '\0'; ulIdx++)
                    {
                    }

                    /*
                     * Write the string.
                     */
                    for(temp = 0; temp < ulIdx; temp++)
                    {
                        *out = pcStr[temp];
                        out++;
                    }

                    /*
                     * Write any required padding spaces
                     */
                    if(ulCount > ulIdx)
                    {
                        ulCount -= ulIdx;
                        while(ulCount--)
                        {
                            *out = ' ';
                            out++;
                        }
                    }

                    /*
                     * This command has been handled.
                     */
                    break;
                }

                /*
                 * Handle the %u command.
                 */
                case 'u':
                {
                    /*
                     * Get the value from the varargs.
                     */
                    ulValue = va_arg(vaArgP, unsigned long);

                    /*
                     * Reset the buffer position.
                     */
                    ulPos = 0;

                    /*
                     * Set the base to 10.
                     */
                    ulBase = 10;

                    /*
                     * Indicate that the value is positive so that a minus sign
                     * isn't inserted.
                     */
                    ulNeg = 0;

                    /*
                     * Convert the value to ASCII.
                     */
                    goto convert;
                }

                /*
                 * Handle the %x and %X commands.  Note that they are treated
                 * identically; i.e. %X will use lower case letters for a-f
                 * instead of the upper case letters is should use.  We also
                 * alias %p to %x.
                 */
                case 'x':
                case 'X':
                case 'p':
                {
                    /*
                     * Get the value from the varargs.
                     */
                    ulValue = va_arg(vaArgP, unsigned long);

                    /*
                     * Reset the buffer position.
                     */
                    ulPos = 0;

                    /*
                     * Set the base to 16.
                     */
                    ulBase = 16;

                    /*
                     * Indicate that the value is positive so that a minus sign
                     * isn't inserted.
                     */
                    ulNeg = 0;

                    /*
                     * Determine the number of digits in the string version of
                     * the value.
                     */
convert:
                    for(ulIdx = 1;
                        (((ulIdx * ulBase) <= ulValue) &&
                         (((ulIdx * ulBase) / ulBase) == ulIdx));
                        ulIdx *= ulBase, ulCount--)
                    {
                    }

                    /*
                     * If the value is negative, reduce the count of padding
                     * characters needed.
                     */
                    if(ulNeg)
                    {
                        ulCount--;
                    }

                    /*
                     * If the value is negative and the value is padded with
                     * zeros, then place the minus sign before the padding.
                     */
                    if(ulNeg && (cFill == '0'))
                    {
                        /*
                         * Place the minus sign in the output buffer.
                         */
                        pcBuf[ulPos++] = '-';

                        /*
                         * The minus sign has been placed, so turn off the
                         * negative flag.
                         */
                        ulNeg = 0;
                    }

                    /*
                     * Provide additional padding at the beginning of the
                     * string conversion if needed.
                     */
                    if((ulCount > 1) && (ulCount < 16))
                    {
                        for(ulCount--; ulCount; ulCount--)
                        {
                            pcBuf[ulPos++] = cFill;
                        }
                    }

                    /*
                     * If the value is negative, then place the minus sign
                     * before the number.
                     */
                    if(ulNeg)
                    {
                        /*
                         * Place the minus sign in the output buffer.
                         */
                        pcBuf[ulPos++] = '-';
                    }

                    /*
                     * Convert the value into a string.
                     */
                    for(; ulIdx; ulIdx /= ulBase)
                    {
                        pcBuf[ulPos++] = g_pcHex[(ulValue / ulIdx) % ulBase];
                    }

                    /*
                     * Write the string.
                     */
                    for(temp = 0; temp < ulPos; temp++)
                    {
                        *out = pcBuf[temp];
                        out++;
                    }

                    /*
                     * This command has been handled.
                     */
                    break;
                }

                /*
                 * Handle the %% command.
                 */
                case '%':
                {
                    /*
                     * Simply write a single %.
                     */
                    *out = '%';
                    out++;

                    /*
                     * This command has been handled.
                     */
                    break;
                }

                /*
                 * Handle all other commands.
                 */
                default:
                {
                    /*
                     * Indicate an error.
                     */

                    /*
                     * This command has been handled.
                     */
                    break;
                }
            }
        }
    }
}


void DEFAULT_SPRINTF(char *out, const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    sg_varargs(out, format, argptr);
    va_end(argptr);
}


/*
 * This method causes the current thread to wait for "n" seconds.
 */
void startAndCountdownTimer(int seconds, unsigned char showRunningStatus)
{
    int i;
    long j;
    long cycles = 1000000 / getMinimumDelayPossibleInMicroSeconds();

    for(i = 0; i < seconds; i++)
    {
        if(showRunningStatus == 1)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "[%u]", seconds - i);
            info_log(LOG_GLOBAL_BUFFER);
        }

        for(j = 0; j < cycles; j++)
        {
            minimumDelay();
        }
    }
}


static void set_up_network_ports()
{
#if SSL_ENABLED == 1
    {
        int rc = get_config_value_from_persistent_storage(SSL_ACTUALLY_ENABLED, (char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));

        sslEnabledAtAppLayer = 0;
        if(rc == SUCCESS)
        {
            char small[3] = {0};
            getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, small);

            sslEnabledAtAppLayer = sg_atoi(small);
        }
    }
#else
    sslEnabledAtAppLayer = 0;
#endif


#if SOCKET_SSL_ENABLED == 1
    {
        int rc = get_config_value_from_persistent_storage(SSL_ACTUALLY_ENABLED, (char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));

        sslEnabledAtSocketLayer = 1;
        if(rc == SUCCESS)
        {
            char small[3] = {0};
            getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, small);

            sslEnabledAtSocketLayer = sg_atoi(small);
        }
    }
#else
    sslEnabledAtSocketLayer = 0;
#endif


    if((sslEnabledAtAppLayer == 0) && (sslEnabledAtSocketLayer == 0))
    {
        INSTAMSG_PORT = 1883;
        INSTAMSG_HTTP_PORT = 80;
    }
    else
    {
        INSTAMSG_PORT = 8883;
        INSTAMSG_HTTP_PORT = 443;
    }
}


void globalSystemInit(char *logFilePath)
{
    debugLoggingEnabled = 1;

    bootstrapInit();

    currentLogLevel = LOG_LEVEL;
    init_serial_logger();

    init_global_timer();
    init_watchdog();

#if FILE_LOGGING_ENABLED == 1
    init_file_logger(&fileLogger, logFilePath);
#endif

#if AT_INTERFACE_ENABLED == 1
    init_at_interface();
#endif

    init_config();
    init_data_logger();

    set_up_network_ports();

    sg_sprintf(LOG_GLOBAL_BUFFER, "\n\nInstamsg-Version ====> [%s]\n\n", INSTAMSG_VERSION);
    info_log(LOG_GLOBAL_BUFFER);
}


char LOG_GLOBAL_BUFFER[MAX_BUFFER_SIZE];
unsigned char GLOBAL_BUFFER[MAX_BUFFER_SIZE];
int pingRequestInterval;
int compulsorySocketReadAfterMQTTPublishInterval;
#if MEDIA_STREAMING_ENABLED == 1
int mediaStreamingEnabledRuntime;
#endif

char messageBuffer[MAX_BUFFER_SIZE];


#if MEDIA_STREAMING_ENABLED == 1
unsigned char mediaStreamingErrorOccurred;
#endif

#ifdef DEBUG_MODE
char USER_LOG_FILE_PATH[MAX_BUFFER_SIZE];
char USER_DEVICE_UUID[MAX_BUFFER_SIZE];
#endif

volatile int editableBusinessLogicInterval;
volatile int countdownTimerForBusinessLogic;

int debugLoggingEnabled;

int INSTAMSG_PORT;
int INSTAMSG_HTTP_PORT;
int sslEnabledAtAppLayer;
int sslEnabledAtSocketLayer;

