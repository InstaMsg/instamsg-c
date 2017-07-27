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



#include "./include/gpio.h"
#include "./include/sg_stdlib.h"

#if SEND_GPIO_INFORMATION == 1

#include <string.h>

void fillPinPrefix(char *orientationConfig, int positionInOrientationConfig, char *pinPrefix, char *pinId)
{
    char orientation[10] = {0};

    memset(orientation, 0, sizeof(orientation));
    get_nth_token_thread_safe(orientationConfig, ',', positionInOrientationConfig, orientation, 1);

    if(strcmp(orientation, "in") == 0)
    {
        sg_sprintf(pinPrefix, "DI%s", pinId);
    }
    else if(strcmp(orientation, "out") == 0)
    {
        sg_sprintf(pinPrefix, "DO%s", pinId);
    }
}

#endif

