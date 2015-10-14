#include <string.h>

#include "include/log.h"
#include "include/sg_mem.h"

static unsigned long currentBytesUsed;

uintptr_t sg_malloc(unsigned short numBytes)
{
    void *mem = NULL;

    if((currentBytesUsed + numBytes + HEADER_SIZE) > MAX_HEAP_SIZE)
    {
        error_log(MEM_ALLOC "Cannot allocate [%u] bytes of memory.", numBytes);
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
    debug_log(MEM_ALLOC "Current memory remaining in bytes = [%u]", MAX_HEAP_SIZE - currentBytesUsed);

exit:
    return ((uintptr_t) mem);
}


void sg_free(void *ptr)
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
}


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

