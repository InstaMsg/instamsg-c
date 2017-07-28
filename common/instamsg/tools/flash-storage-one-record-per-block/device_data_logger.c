/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include "../../../common/instamsg/driver/include/globals.h"
#include "../../../common/instamsg/driver/include/log.h"

#include "../common/storage_utils.h"

#include <string.h>

#define DATA_TYPE                       "DATA"


/**********************************************************************************
 *
 * TODO: ADD/REPLACE THIS FILE AT device/name/instamsg/device_data_logger.c
 *
 *********************************************************************************/


static void write_next_read_and_write_record_number_values(unsigned short readNumber, unsigned short writeNumber)
{
    if(writeNumber == (DATA_RECORDS_UPPER_INDEX + 1))
    {
        writeNumber = FIRST_RECORD_NUMBER_VALUE;
    }
    if(readNumber == (DATA_RECORDS_UPPER_INDEX + 1))
    {
        readNumber = FIRST_RECORD_NUMBER_VALUE;
    }

    write_mbr(readNumber, writeNumber);
}


/*
 * This method initializes the data-logger-interface for the device.
 */
void init_data_logger()
{
    init_persistent_storage();
}


/*
 * This method saves the record on the device.
 *
 * If and when the device-storage becomes full, the device MUST delete the oldest record, and instead replace
 * it with the current record. That way, we will maintain a rolling-data-logger.
 */
void save_record_to_persistent_storage(char *record)
{
    unsigned short nextReadNumber, nextWriteNumber;

    read_next_read_and_write_record_number_values(&nextReadNumber, &nextWriteNumber);
    write_record_on_persistent_storage(nextWriteNumber, record, DATA_TYPE);

    /*
     * If we have just written the record which was actually about to be read next, that means it has been overwritten.
     * Advance the read-pointer.
     */
    if(nextWriteNumber == nextReadNumber)
    {
        nextReadNumber++;
    }

    /*
     * If till this point, there was no record to be read, so point the read-pointer to the just-written record.
     */
    if(nextReadNumber == NO_MORE_RECORD_TO_READ)
    {
        nextReadNumber = nextWriteNumber;
    }

    nextWriteNumber++;
    write_next_read_and_write_record_number_values(nextReadNumber, nextWriteNumber);
}


/*
 * The method returns the next available record.
 * If a record is available, following must be done ::
 *
 * 1)
 * The record must be deleted from the storage-medium (so as not to duplicate-fetch this record later).
 *
 * 2)
 * Then actually return the record.
 *
 * Obviously, there is a rare chance that step 1) is finished, but step 2) could not run to completion.
 * That would result in a data-loss, but we are ok with it, because we don't want to send duplicate-records to InstaMsg-Server.
 *
 * We could have done step 2) first and then step 1), but in that scenario, we could have landed in a scenario where step 2)
 * was done but step 1) could not be completed. That could have caused duplicate-data on InstaMsg-Server, but we don't want
 * that.
 *
 *
 * One of the following statuses must be returned ::
 *
 * a)
 * SUCCESS, if a record is successfully returned.
 *
 * b)
 * FAILURE, if no record is available.
 */

#define NO_RECORD_TO_READ_CODE                                                              \
    sg_sprintf(LOG_GLOBAL_BUFFER, "No more records to read from persistent-storage.");      \
    info_log(LOG_GLOBAL_BUFFER);                                                            \
                                                                                            \
    nextReadNumber = NO_MORE_RECORD_TO_READ;                                                \
    rc = FAILURE;                                                                           \
                                                                                            \
    goto exit;

int get_next_record_from_persistent_storage(char *buffer, int maxLength)
{
    int rc;
    unsigned short nextReadNumber, nextWriteNumber;

    read_next_read_and_write_record_number_values(&nextReadNumber, &nextWriteNumber);
    if(nextReadNumber == NO_MORE_RECORD_TO_READ)
    {
        NO_RECORD_TO_READ_CODE
    }

    /*
     * Check if the record is valid, and proceed only if it is.
     */
    if(is_record_valid(nextReadNumber) == FAILURE)
    {
        NO_RECORD_TO_READ_CODE
    }

    /*
     * If we reach till here, read the actual record.
     */
    read_record_from_persistent_storage(nextReadNumber, buffer, maxLength, DATA_TYPE);

    /*
     * Mark the just-read record invalid.
     */
    mark_record_invalid(nextReadNumber);

    /*
     * Finally, move the read-pointer, to (try and ) read the next-record.
     */
    nextReadNumber++;
    rc = SUCCESS;

exit:
    write_next_read_and_write_record_number_values(nextReadNumber, nextWriteNumber);
    return rc;
}


/*
 * This method releases the data-logger, just before the system is going for a reset.
 */
void release_data_logger()
{
}
