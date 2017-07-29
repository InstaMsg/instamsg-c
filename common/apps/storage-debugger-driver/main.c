#include "../../instamsg/driver/include/globals.h"
#include "../../instamsg/driver/include/log.h"
#include "../../instamsg/driver/include/data_logger.h"

#include "device_defines.h"

#include <string.h>


char buf[MAX_BUFFER_SIZE];


void release_app_resources()
{
}


int main(int argc, char** argv)
{
    int rc = FAILURE;


    init_data_logger();

    /*
     * Do as many calls to save_record_to_persistent_storage ...
     */
    /* void save_record_to_persistent_storage(char *record) */


    while(1)
    {
        memset(buf, 0, sizeof(buf));

        rc = get_next_record_from_persistent_storage(buf, sizeof(buf));
        if(rc == SUCCESS)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "Record read from persistent-storage = [%s]", buf);
            info_log(LOG_GLOBAL_BUFFER);
        }
        else
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "No more records available in persistent-storage");
            info_log(LOG_GLOBAL_BUFFER);

            break;
        }
    }

    return 0;
}
