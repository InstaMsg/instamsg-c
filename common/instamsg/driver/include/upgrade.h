#ifndef FILE_UPGRADE_INSTAMSG
#define FILE_UPGRADE_INSTAMSG

#define FILE_UPGRADE                PROSTR("[FILE_UPGRADE] ")
#define NEW_FILE_KEY                PROSTR("NEW_FILE_ARRIVED")
#define NEW_FILE_ARRIVED            PROSTR("YES")

void prepare_for_new_binary_download();
void copy_next_char(char c);
void tear_down_binary_download();

void check_for_upgrade();
void upgrade_executable_binary();

#endif
