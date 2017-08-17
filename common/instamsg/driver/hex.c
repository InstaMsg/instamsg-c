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

#include "./include/hex.h"

#include <string.h>

int getIntValueOfHexChar(char c)
{
    if((c == 'a') || (c == 'A'))
    {
        return 10;
    }

    if((c == 'b') || (c == 'B'))
    {
        return 11;
    }

    if((c == 'c') || (c == 'C'))
    {
        return 12;
    }

    if((c == 'd') || (c == 'D'))
    {
        return 13;
    }

    if((c == 'e') || (c == 'E'))
    {
        return 14;
    }

    if((c == 'f') || (c == 'F'))
    {
        return 15;
    }

    return (c - 48);
}


void getByteStreamFromHexString(const char *hexString, unsigned char *buffer)
{
    unsigned int i, j;

    for(i = 0, j = 0; i < strlen(hexString); i = i + 2, j++)
    {
        int ascii;

        char buf[3] = {0};
        memcpy(buf, hexString + i, 2);


        ascii = 0;
        ascii = ascii + (16 * getIntValueOfHexChar(buf[0]));
        ascii = ascii + getIntValueOfHexChar(buf[1]);

        buffer[j] = ascii;
    }
}


void addPaddingIfRequired(char *buf, int size)
{
    int i, j;

    for(i = size - 1, j = strlen(buf) - 1; j >= 0; i--, j--)
    {
        buf[i] = buf[j];
    }

    for(; i >= 0; i--)
    {
        buf[i] = '0';
    }
}

