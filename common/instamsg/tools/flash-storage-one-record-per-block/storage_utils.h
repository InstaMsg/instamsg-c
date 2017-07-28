#ifndef DEVICE_STORAGE_UTILS
#define DEVICE_STORAGE_UTILS


/**********************************************************************************
 *
 * TODO: ADD/REPLACE THIS FILE AT device/name/common/storage_utils.h
 *
 *********************************************************************************/


/*
 * Only these need to be changed if the persistent-storage capacity/sizes are changed.
 */
#define MEMORY_SIZE_BYTES                       TODO  /* Eg ==> (1 * 1024 * 1024) */
#define MAX_RECORD_SIZE_BYTES                   TODO  /* Eg ==> 4096              */


#define MAX_RECORDS                             (MEMORY_SIZE_BYTES / MAX_RECORD_SIZE_BYTES)


#define CONFIG_RECORDS_LOWER_INDEX              1
#define CONFIG_RECORDS_UPPER_INDEX              50

#define DATA_RECORDS_LOWER_INDEX                51
#define DATA_RECORDS_UPPER_INDEX                (MAX_RECORDS - 20)

#define NO_MORE_RECORD_TO_READ                  (MAX_RECORDS - 5)
#define FIRST_RECORD_NUMBER_VALUE               DATA_RECORDS_LOWER_INDEX

void init_persistent_storage();
void write_mbr(unsigned short readNumber, unsigned short writeNumber);
void read_next_read_and_write_record_number_values(unsigned short *readNumber, unsigned short *writeNumber);
int is_record_valid(unsigned short recordNumber);
void mark_record_invalid(unsigned short recordNumber);
int read_record_from_persistent_storage(unsigned short recordNumber, char *buffer, int maxBufferLength, const char *recordType);
int write_record_on_persistent_storage(unsigned short recordNumber, const char *record, const char *recordType);

#endif
