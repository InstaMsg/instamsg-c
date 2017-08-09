#ifndef DEVICE_PROXY
#define DEVICE_PROXY

#include "device_defines.h"

#include "../../../common/instamsg/driver/include/instamsg.h"
#include "../../../common/instamsg/driver/include/misc.h"
#include "../../../common/instamsg/driver/include/file_system.h"
#include "../../../common/instamsg/driver/include/config.h"
#include "../../../common/instamsg/driver/include/proxy.h"

#if HTTP_PROXY_ENABLED == 1


void setupProxy(InstaMsg *c)
{
    char command[500] = {0};
    int rc = FAILURE;

    RESET_GLOBAL_BUFFER;

    rc = get_config_value_from_persistent_storage(PROXY_END_SERVER_PORT, (char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));
    if(rc == SUCCESS)
    {
        char value[200] = {0};
        getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, value);

        if(strlen(value) > 0)
        {
            char *subs = "http://";
            char *ptr = strstr(value, subs);

            if(ptr != NULL)
            {
                sg_sprintf(command, "%s %s %s %s %s",
                           c->proxyPasswd, c->proxyPort, value + strlen(subs), c->proxyHttpUser, c->proxyServer);
            }
            else
            {
                sg_sprintf(command, "%s %s %s %s %s",
                           c->proxyPasswd, c->proxyPort, value + strlen("https://"), c->proxyHttpsUser, c->proxyServer);
            }
        }
    }


    if(strlen(command) > 0)
    {
        sg_writeFile(SYSTEM_WIDE_TEMP_FILE, command);
        sg_renameFile(NULL, SYSTEM_WIDE_TEMP_FILE, "proxy_command");
    }
}

#endif

#endif
