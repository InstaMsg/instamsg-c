#include "./storage_utils.h"


/**********************************************************************************
 *
 * TODO: ADD/REPLACE THIS FILE AT device/name/common/storage_utils.c
 *
 *********************************************************************************/

#include <string.h>

#include "../../../common/instamsg/driver/include/log.h"
#include "../../../common/instamsg/driver/include/globals.h"
#include "../../../common/instamsg/driver/include/misc.h"

#define INVALID_SIZE														0
#define OPERATING_SECTOR_SIZE										        1024

#define MEMORY_FORMATTED_IDENTIFIER                                         'y'

volatile unsigned char persistent_storage_initialized;
static unsigned char tempBuffer[MAX_RECORD_SIZE_BYTES];


/*
 *
 * ALL DEVICE-SPECIFIC CODE GOES HERE NOW ...
 *
 */


/**********************************************************************************************************
 **************************   BOUNDARY WALL BETWEEN LOWER AND MIDDLE LAYER ********************************
 *********************************************************************************************************/

static void write_record_at_address(unsigned long address, unsigned char *buffer, int len)
{
    /* TODO: CORE RECORD-WRITE CODE GOES HERE */
}


static void read_record_at_address(unsigned long address, unsigned char *buffer, int len)
{
    /* TODO: CORE RECORD-READ CODE GOES HERE */
}


static void spi_flash_init()
{
    static unsigned char flash_init_done = 0;

    if(flash_init_done == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "SPI-Flash has already been initialized, not re-initializing ...");
        info_log(LOG_GLOBAL_BUFFER);

        return;
    }

    /*
     *
     *  TODO: INSERT FLASH-INIT CODE HERE
     *
     */

    flash_init_done = 1;
}


static void write_record(int recordNumber, unsigned char *buffer, unsigned short len)
{
    unsigned char *tmp = tempBuffer;

    if(len >= (OPERATING_SECTOR_SIZE - 10))
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Maximum alllowed size exceeded");
        error_log(LOG_GLOBAL_BUFFER);

        resetDevice();
    }

    /*
     * Write the length of the record in two bytes itself at the start of record.
     */
    {
        int i = 2, j = 0;
        unsigned char *ptr = (unsigned char*) &len;

        tmp[0] = *ptr;
        tmp[1] = *(ptr + 1);

        for(i = 2, j = 0; j < len; i++, j++)
        {
            tmp[i] = buffer[j];
        }

        write_record_at_address(recordNumber * MAX_RECORD_SIZE_BYTES, tmp, len + 2);
    }
}


static unsigned short read_record(int recordNumber, unsigned char *buffer)
{
    unsigned char *tmp = tempBuffer;

    unsigned short len = 0;

    /*
     * Read the length of the record, stored in the first two bytes of the record itself.
     */
    {
        unsigned char *ptr = (unsigned char*) &len;

        read_record_at_address(recordNumber * MAX_RECORD_SIZE_BYTES, tmp, OPERATING_SECTOR_SIZE);
        *ptr        = tmp[0];
        *(ptr + 1)  = tmp[1];

        memcpy(buffer, tmp + 2, len);
        return len;
    }

    return len;
}


static unsigned short getRecordSize(unsigned short recordNumber)
{
    unsigned short c;
    unsigned char *ptr = (unsigned char*) &c;

    /*
     * Using LOG_GLOBAL_BUFFER to save RAM.
     */
    read_record_at_address(recordNumber * MAX_RECORD_SIZE_BYTES, (unsigned char*)LOG_GLOBAL_BUFFER, 2);

    *ptr        = ((unsigned char*)LOG_GLOBAL_BUFFER)[0];
    *(ptr + 1) 	= ((unsigned char*)LOG_GLOBAL_BUFFER)[1];

    memset(LOG_GLOBAL_BUFFER, 0, sizeof(LOG_GLOBAL_BUFFER));
    return c;
}


void write_mbr(unsigned short readNumber, unsigned short writeNumber)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, "Writing readNumber [%u], writeNumber [%u]", readNumber, writeNumber);
    debug_log(LOG_GLOBAL_BUFFER);

    {
        unsigned char small[20] = {0};
        unsigned char *ptr = (unsigned char*) &readNumber;

        small[0] = MEMORY_FORMATTED_IDENTIFIER;

        small[1] = *ptr;
        small[2] = *(ptr + 1);

        ptr = (unsigned char*) &writeNumber;
        small[3] = *ptr;
        small[4] = *(ptr + 1);

        write_record(0, small, 5);
    }

    sg_sprintf(LOG_GLOBAL_BUFFER, "Done\n");
    info_log(LOG_GLOBAL_BUFFER);
}


void read_next_read_and_write_record_number_values(unsigned short *readNumber, unsigned short *writeNumber)
{
    unsigned char *ptr;
    unsigned char small[10] = {0};

    read_record(0, small);

    ptr = (unsigned char*) readNumber;
    *ptr        = small[1];
    *(ptr + 1)  = small[2];

    ptr = (unsigned char*) writeNumber;
    *ptr        = small[3];
    *(ptr + 1)  = small[4];

    sg_sprintf(LOG_GLOBAL_BUFFER, "Reading readNumber [%u], writeNumber [%u]", *readNumber, *writeNumber);
    debug_log(LOG_GLOBAL_BUFFER);
}


int is_record_valid(unsigned short recordNumber)
{
    if(getRecordSize(recordNumber) == INVALID_SIZE)
    {
        return FAILURE;
    }

    return SUCCESS;
}


void mark_record_invalid(unsigned short recordNumber)
{
    unsigned char tmp[1] = {0};
    write_record(recordNumber, tmp, INVALID_SIZE);
}


int read_record_from_persistent_storage(unsigned short recordNumber, char *buffer, int maxBufferLength, const char *recordType)
{
    read_record(recordNumber, (unsigned char*)buffer);
    return SUCCESS;
}


int write_record_on_persistent_storage(unsigned short recordNumber, const char *record, const char *recordType)
{
    write_record(recordNumber, (unsigned char *)record, strlen(record));
    return SUCCESS;
}


void init_persistent_storage()
{
    if(persistent_storage_initialized == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Persistent-Storage already initialized !!");
        info_log(LOG_GLOBAL_BUFFER);

        return;
    }

    spi_flash_init();

     /*
      * We use the first-record of SPI-FLash as the MBR.
      *
      * Check if the first-byte is a 'y'.
      * If it is, it means that the memory has been initialized once fine.
      */

    read_record(0, (unsigned char*)LOG_GLOBAL_BUFFER);

    {
        unsigned char byte = LOG_GLOBAL_BUFFER[0];
        memset(LOG_GLOBAL_BUFFER, 0, sizeof(LOG_GLOBAL_BUFFER));

        if(byte == MEMORY_FORMATTED_IDENTIFIER)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "Flash-Memory has already been formatted once at beginning of its universe.");
            info_log(LOG_GLOBAL_BUFFER);

            persistent_storage_initialized = 1;
            return;
        }
        else
        {
            int i = 0;

            sg_sprintf(LOG_GLOBAL_BUFFER, "Character read is [%u], Flash-Memory has NOT been formatted. Doing ...", byte);
            info_log(LOG_GLOBAL_BUFFER);

            startAndCountdownTimer(3, 1);

            /*
             * Erase the data-pages.
             */
            for(i = 1; i < MAX_RECORDS; i++)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, "Erasing Page [%u]", i);
                info_log(LOG_GLOBAL_BUFFER);

                mark_record_invalid(i);
            }

            write_mbr(NO_MORE_RECORD_TO_READ, FIRST_RECORD_NUMBER_VALUE);
            persistent_storage_initialized = 1;
        }
    }
}
