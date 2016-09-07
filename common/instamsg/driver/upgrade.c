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


#include "./include/upgrade.h"
#include "./include/sg_mem.h"
#include "./include/config.h"
#include "./include/json.h"
#include "./include/log.h"
#include "./include/misc.h"

#include <string.h>

void check_for_upgrade()
{
    int rc = FAILURE;
    char *new_file_arrived_config = NULL;
    unsigned char reboot = 0;

    RESET_GLOBAL_BUFFER;

    rc = get_config_value_from_persistent_storage(NEW_FILE_KEY, (char*) GLOBAL_BUFFER, MAX_BUFFER_SIZE);
    if(rc == FAILURE)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sConfig not found on persistent storage ... so proceeding with old file"), FILE_UPGRADE);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }

    new_file_arrived_config = (char*) sg_malloc(MAX_BUFFER_SIZE);
    if(new_file_arrived_config == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER,
                   PROSTR("%sCould not allocate memory for checking whether new-file arrived ... so proceeding with old file"), FILE_UPGRADE);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }

    memset(new_file_arrived_config, 0, MAX_BUFFER_SIZE);
    getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, new_file_arrived_config);

    if(strcmp(new_file_arrived_config, NEW_FILE_ARRIVED) == 0)
    {
        remove_old_executable_binary();
        copy_new_executable_binary_from_temp_location();

        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sBinary upgraded, restarting to take effect"), FILE_UPGRADE);
        info_log(LOG_GLOBAL_BUFFER);

        reboot = 1;
    }

    delete_config_value_from_persistent_storage(NEW_FILE_KEY);

exit:
    if(new_file_arrived_config)
    {
        sg_free(new_file_arrived_config);
    }

    if(reboot == 1)
    {
        exitApp();
    }
}

