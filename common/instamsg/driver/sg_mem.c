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

#include "include/log.h"
#include "include/sg_mem.h"

#if USE_DEFAULT_MALLOC == 1

static char sg_heap[MAX_HEAP_SIZE];
static unsigned int currentBytesUsed;

static int count;
static int maxAllocationsCount;

void* DEFAULT_MALLOC(unsigned short numBytes)
{
    static unsigned char oneCallDone = 0;
    void *mem = NULL;

    if(oneCallDone == 0)
    {
        oneCallDone = 1;
        currentBytesUsed = 0;
    }


    if((currentBytesUsed + numBytes + HEADER_SIZE) > MAX_HEAP_SIZE)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCannot allocate [%u] bytes of memory."), MEM_ALLOC, numBytes);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }

    /*
     * Copy the bytes to be allocated in the header.
     */
    memcpy(sg_heap + currentBytesUsed, &numBytes, HEADER_SIZE);
    currentBytesUsed = currentBytesUsed + HEADER_SIZE;


    /*
     * Now, allocate the data-bytes.
     *
     * The only thing really needed is to memset all the bytes to 0.
     * Overflows don't need to be taken care of here, it is the responsibility of the client.
     */
    memset(sg_heap + currentBytesUsed, 0, numBytes);

    /*
     * Assign the starting-pointer of the memory-block to be returned.
     */
    mem = sg_heap + currentBytesUsed;

    /*
     * Set the global counter, to keep track of the area to be used for next memory-allocation.
     */
    currentBytesUsed = currentBytesUsed + numBytes;

    count++;
    if(count > maxAllocationsCount)
    {
        maxAllocationsCount = count;

        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR(" ******************************** MAX ALLOCATIONS SO FAR [%u]"), maxAllocationsCount);
        info_log(LOG_GLOBAL_BUFFER);
    }

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCurrent memory remaining in bytes = [%u]"), MEM_ALLOC, MAX_HEAP_SIZE - currentBytesUsed);
    debug_log(LOG_GLOBAL_BUFFER);

exit:
    return mem;
}


void DEFAULT_FREE(void *ptr)
{
    /*
     * Extract the number of bytes allocated to this block.
     */
    unsigned short numBytes;

    if(ptr != NULL)
    {
        memcpy(&numBytes, (char*)ptr - HEADER_SIZE, HEADER_SIZE);

        /*
        * Now, simply reverse the counter keeping track of the bytes allocated from the heap.
        */
        currentBytesUsed = currentBytesUsed - numBytes - HEADER_SIZE;
    }

    count--;
}
#endif

char *sg_mem_strstr(char *string, const char *substring, int len)
{
    register const char *a, *b;

    /* First scan quickly through the two strings looking for a
     * single-character match.  When it's found, then compare the
     * rest of the substring.
     */

    b = substring;
    if (*b == 0) {
        return string;
    }

    for ( ; len != 0; string += 1, len--)
    {
        if (*string != *b)
        {
            continue;
        }

        a = string;
        while (1)
        {
            if (*b == 0)
            {
                return string;
            }

            if (*a++ != *b++)
            {
                break;
            }
        }

        b = substring;
    }

    return (char *) 0;
}

