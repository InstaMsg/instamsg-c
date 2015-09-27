#include "../instamsg/driver/include/instamsg.h"
#include "../instamsg/driver/include/globals.h"


static int sendClientSessionData()
{
    return SUCCESS;
}


static int sendClientMetaData()
{
    return SUCCESS;
}


static int onConnect()
{
    int rc = FAILURE;

    rc = sendClientSessionData();
    rc = sendClientMetaData();

    return rc;
}


void coreLoopyBusinessLogicInitiatedBySelf()
{
    onConnect();
    startAndCountdownTimer(300, 1);
}


int main(int argc, char** argv)
{
    globalSystemInit();
    start(onConnect, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, NULL);
}

