#ifndef IOEYE_DATA_LOGGER
#define IOEYE_DATA_LOGGER


#define DATA_LOGGING            PROSTR("[DATA-LOGGING] ")
#define DATA_LOGGING_ERROR      PROSTR("[DATA-LOGGING-ERROR] ")

void init_data_logger();
void save_record_to_persistent_storage(char *record);
int get_next_record_from_persistent_storage(char *buffer, int maxLength);

#endif
