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
#define MEMORY_SIZE_BYTES                       (2 * 1024 * 1024)       /*      2 MB Flash          */
#define MAX_RECORD_SIZE_BYTES                   (4 * 1024)              /*      4 KB Block-Size     */


#define MBR_INDEX								0
#define MAX_RECORDS								((MEMORY_SIZE_BYTES) / (MAX_RECORD_SIZE_BYTES))

#define CONFIG_RECORDS_LOWER_INDEX              (MBR_INDEX + 1)
#define CONFIG_RECORDS_UPPER_INDEX              20

#define DATA_RECORDS_LOWER_INDEX                21
#define DATA_RECORDS_UPPER_INDEX                (MAX_RECORDS - 4)

#define NO_MORE_RECORD_TO_READ                  (MAX_RECORDS - 2)
#define FIRST_RECORD_NUMBER_VALUE               DATA_RECORDS_LOWER_INDEX

void init_persistent_storage(void);
void write_mbr(unsigned short readNumber, unsigned short writeNumber);
void read_next_read_and_write_record_number_values(unsigned short *readNumber, unsigned short *writeNumber);
int is_record_valid(unsigned short recordNumber);
void mark_record_invalid(unsigned short recordNumber);
int read_record_from_persistent_storage(unsigned short recordNumber, char *buffer, int maxBufferLength, const char *recordType);
int write_record_on_persistent_storage(unsigned short recordNumber, const char *record, const char *recordType);

#endif
