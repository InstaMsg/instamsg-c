/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#define TEMP_FILE_NAME      "~instamsg"
#define BINARY_NAME         "instamsg"

#include "../../../common/instamsg/driver/include/upgrade.h"
#include "../../../common/instamsg/driver/include/instamsg.h"
#include "../../../common/instamsg/driver/include/log.h"
#include "../../../common/instamsg/driver/include/file_system.h"

static FileSystem fs;

void prepare_for_new_binary_download()
{
    instaMsg.singletonUtilityFs.deleteFile(&(instaMsg.singletonUtilityFs), TEMP_FILE_NAME);
    init_file_system(&fs, TEMP_FILE_NAME);
}


void copy_next_char(char c)
{
    fs.write(&fs, (unsigned char*)&c, 1);
}


void tear_down_binary_download()
{
    release_file_system(&fs);
}


void remove_old_executable_binary()
{
    instaMsg.singletonUtilityFs.deleteFile(&(instaMsg.singletonUtilityFs), BINARY_NAME);

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sOld Binary [%s] successfully deleted."), FILE_UPGRADE, BINARY_NAME);
    info_log(LOG_GLOBAL_BUFFER);
}


void copy_new_executable_binary_from_temp_location()
{
    instaMsg.singletonUtilityFs.copyFile(&(instaMsg.singletonUtilityFs), TEMP_FILE_NAME, BINARY_NAME);

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFile [%s] successfully copied to [%s]"), FILE_UPGRADE, TEMP_FILE_NAME, BINARY_NAME);
    info_log(LOG_GLOBAL_BUFFER);
}
