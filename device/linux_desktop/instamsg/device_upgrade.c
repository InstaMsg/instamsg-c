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


void upgrade_executable_binary()
{
    instaMsg.singletonUtilityFs.deleteFile(&(instaMsg.singletonUtilityFs), BINARY_NAME);
    instaMsg.singletonUtilityFs.copyFile(&(instaMsg.singletonUtilityFs), TEMP_FILE_NAME, BINARY_NAME);

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFile [%s] successfully moved to [%s]"), FILE_UPGRADE, TEMP_FILE_NAME, BINARY_NAME);
    info_log(LOG_GLOBAL_BUFFER);
}
