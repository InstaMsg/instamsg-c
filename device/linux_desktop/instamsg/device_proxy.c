#ifndef DEVICE_PROXY
#define DEVICE_PROXY

#include "device_defines.h"

#include "../../../common/instamsg/driver/include/instamsg.h"
#include "../../../common/instamsg/driver/include/misc.h"

#if HTTP_PROXY_ENABLED == 1

static void *setupTunnel(void *arg)
{
    char command[500] = {0};
    InstaMsg *c = (InstaMsg *) arg;

    sg_sprintf(command, "sshpass -p %s ssh -R 0.0.0.0:%s:%s %s@%s",
                        c->proxyPasswd, c->proxyPort, c->proxyEndUnitServerAndPort, c->proxyUser, c->proxyServer);

    sg_sprintf(LOG_GLOBAL_BUFFER, "Command for proxy = [%s]", command);
    info_log(LOG_GLOBAL_BUFFER);

    system(command);
    resetDevice();
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
