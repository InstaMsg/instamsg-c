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

#include "./include/sg_stdlib.h"

int sg_atoi(const char *buf)
{
    int result, len, i, exponent, k;
    unsigned char someNumberParsed = 0;
    unsigned char negative = 0;

    result = 0;
    exponent = -1;

    len = strlen(buf);
    for(i = len - 1; i >= 0; i--)
    {
        int base = 1;

        if(buf[i] == ' ')
        {
            if(someNumberParsed == 0)
            {
                continue;
            }
            else
            {
                break;
            }
        }
        else if(buf[i] == '-')
        {
            if(someNumberParsed == 0)
            {
            }
            else
            {
                negative = 1;
            }

            continue;
        }
        else if( (buf[i] < '0') || (buf[i] > '9') )
        {
            return 0;
        }

        someNumberParsed = 1;

        exponent++;
        for(k = 0; k < exponent; k++)
        {
            base = base * 10;
        }

        result = result + (base * (buf[i] - '0'));
    }

    if(negative == 1)
    {
        result = result * -1;
    }

    return result;
}


char* sg_strnstr(const char *str1, const char *str2, int maxSize)
{
    int i = 0, j = 0;

    if(str1 == NULL)
    {
        return NULL;
    }
    if(str2 == NULL)
    {
        return NULL;
    }
    if(maxSize < strlen(str2))
    {
        return NULL;
    }

    for(i = 0; i <= (maxSize - ((int)(strlen(str2)))); i++)
    {
        unsigned char failure = 0;
        for(j = 0; j < ((int)(strlen(str2))); j++)
        {
            if(str1[i + j] != str2[j])
            {
                failure = 1;
                break;
            }
        }

        if(failure == 0)
        {
            return ((char*)(str1 + i));
        }
    }

    return NULL;
}


char* sg_memnmem(const char *mem1, const char *mem2, int sizeMem1, int sizeMem2)
{
    int i = 0, j = 0;

    if(mem1 == NULL)
    {
        return NULL;
    }
    if(mem2 == NULL)
    {
        return NULL;
    }
    if(sizeMem1 < sizeMem2)
    {
        return NULL;
    }

    for(i = 0; i <= (sizeMem1 - sizeMem2); i++)
    {
        unsigned char failure = 0;
        for(j = 0; j < sizeMem2; j++)
        {
            if(mem1[i + j] != mem2[j])
            {
                failure = 1;
                break;
            }
        }

        if(failure == 0)
        {
            return ((char*)(mem1 + i));
        }
    }

    return NULL;
}


void get_nth_token_thread_safe(char *original, char separator, int pos, char *res, unsigned char strip)
{
    int buffer_index = 0, num_separators_encountered = 0, token_start_pos = 0, tmp_index = 0;
    int token_end_pos = -1;

    while(1)
    {
    	if(buffer_index == strlen(original))
    	{
            if(num_separators_encountered > 0)
            {
                if(num_separators_encountered == (pos - 1))
                {
                    token_end_pos = buffer_index;
                }
            }
    		break;
    	}

    	if(original[buffer_index] == separator)
    	{
    		num_separators_encountered++;

    		if(num_separators_encountered == pos)
    		{
    			token_end_pos = buffer_index;
    			break;
    		}
    		else
    		{
    			token_start_pos = buffer_index + 1;
    		}
    	}

    	buffer_index++;
    }


    if(token_end_pos == -1)
    {
    	if(num_separators_encountered == 0)
    	{
    		/*
    		 * If no separator found, this means that the whole original-string is the token.
    		 */
    		token_end_pos = strlen(original);
    	}
    	else
    	{
    		return;
    	}
    }

    buffer_index = 0;
    for(tmp_index = token_start_pos; tmp_index < token_end_pos; tmp_index++)
    {
    	res[buffer_index] = original[tmp_index];
    	buffer_index++;
    }

    res[buffer_index] = 0;

    if(strip == 1)
    {
        strip_leading_and_trailing_white_paces(res);
    }
}


void strip_leading_and_trailing_white_paces(char *buffer)
{
	int i = 0;

	/*
	 * Remove the trailing \r and \n (if any).
	 */
	for(i = strlen(buffer) - 1; i >= 0; i--)
	{
		if( (buffer[i] == '\r') || (buffer[i] == '\n') )
		{
			buffer[i] = 0;
		}
		else
		{
			break;
		}
	}

	while( (buffer[0] == '\r') || (buffer[0] == '\n') )
	{
		int length = strlen(buffer);
		for(i = 0; i < length; i++)
		{
			buffer[i] = buffer[i + 1];
		}

		buffer[i] = 0;
	}
}


int get_character_count(char *buffer, char c)
{
    int res = 0, i = 0;
    int len = strlen(buffer);

    for(i = 0; i < len; i++)
    {
        if(buffer[i] == c)
        {
            res++;
        }
    }

    return res;
}


#if 0
int main()
{
}
#endif
