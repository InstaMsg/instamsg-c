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



#ifndef MISC_COMMON_IMPL
#define MISC_COMMON_IMPL

#include "./include/misc.h"
#include "./include/data_logger.h"
#include "./include/config.h"
#include "./include/at.h"
#include "./include/log.h"
#include "./include/time.h"

extern volatile unsigned long nextBusinessLogicTick;
extern volatile int editableBusinessLogicInterval;

void waitBeforeReboot()
{
    if(1)
    {
        long difference = ((long)nextBusinessLogicTick) - ((long)(getCurrentTick()));
        while(difference >= editableBusinessLogicInterval)
        {
            difference = difference - editableBusinessLogicInterval;
        }

        if(difference > 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "Waiting [%u] seconds before rebooting", (int)difference);
            info_log(LOG_GLOBAL_BUFFER);

            startAndCountdownTimer(difference, 1);
        }
    }
}


void exitApp(unsigned char waitForReboot)
{
    if(waitForReboot == 1)
    {
        waitBeforeReboot();
    }

    release_app_resources();

    release_data_logger();
    release_config();

#if AT_INTERFACE_ENABLED == 1
    release_at_interface();
#endif

#if FILE_LOGGING_ENABLED == 1
    release_file_logger(&fileLogger);
#endif

    resetDevice();
}


#endif
