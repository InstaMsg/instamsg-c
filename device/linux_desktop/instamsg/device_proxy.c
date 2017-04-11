#ifndef DEVICE_PROXY
#define DEVICE_PROXY

#include "device_defines.h"

#include "../../../common/instamsg/driver/include/instamsg.h"
#include "../../../common/instamsg/driver/include/misc.h"

#include "../common/file_utils.h"

#if HTTP_PROXY_ENABLED == 1


void setupProxy(InstaMsg *c)
{
    char command[500] = {0};
    sg_sprintf(command, "%s %s %s %s %s",
                        c->proxyPasswd, c->proxyPort, c->proxyEndUnitServerAndPort, c->proxyUser, c->proxyServer);

    sg_writeFile(SYSTEM_WIDE_TEMP_FILE, command);
    renameFile(SYSTEM_WIDE_TEMP_FILE, "proxy_command");
}

#endif

#endif
