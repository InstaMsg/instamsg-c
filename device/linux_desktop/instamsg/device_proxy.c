#ifndef DEVICE_PROXY
#define DEVICE_PROXY

#include "device_defines.h"

#include "../../../common/instamsg/driver/include/instamsg.h"
#include "../../../common/instamsg/driver/include/misc.h"

#include "../common/file_utils.h"

#if HTTP_PROXY_ENABLED == 1

static void *setupTunnel(void *arg)
{
    char command[500] = {0};
    InstaMsg *c = (InstaMsg *) arg;

    sg_sprintf(command, "%s %s %s %s %s",
                        c->proxyPasswd, c->proxyPort, c->proxyEndUnitServerAndPort, c->proxyUser, c->proxyServer);

    sg_writeFile("proxy_command", command);

    while(1)
    {
        sleep(1000);
    }
}


static pthread_t tid;
void setupProxy(InstaMsg *c)
{
    static unsigned char done = 0;

    if(done == 0)
    {
        done = 1;
        pthread_create(&tid, NULL, setupTunnel, c);
    }
}

#endif

#endif
