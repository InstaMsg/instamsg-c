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

#include <string.h>

#include "./include/globals.h"
#include "./include/control.h"
#include "./include/time.h"
#include "./include/json.h"
#include "./include/sg_stdlib.h"

static char temp[20];
int isOkToRunControlCommandTimeWise(char *outerJson)
{
    unsigned long utcTimeStampFromControlCommandLong = 0;

    unsigned long currentTimeStamp = 0;
    int offset = 0;

    int ttl = 0;

    memset(temp, 0, sizeof(temp));
    getJsonKeyValueIfPresent(outerJson, "time", temp);
    utcTimeStampFromControlCommandLong = sg_atoul(temp);

    memset(temp, 0, sizeof(temp));
    getJsonKeyValueIfPresent(outerJson, "ttl", temp);
    ttl = sg_atoi(temp);

    memset(temp, 0, sizeof(temp));
    getTimezoneOffset(temp, sizeof(temp));
    offset = sg_atoi(temp);

    currentTimeStamp = getCurrentTick();
    if( ( currentTimeStamp - ((unsigned long)offset) ) < ( utcTimeStampFromControlCommandLong + ((unsigned long)ttl) ) )
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}
