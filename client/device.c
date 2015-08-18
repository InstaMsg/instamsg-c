#include "src/common/include/instamsg.h"
#include "src/common/include/globals.h"

void coreLoopyBusinessLogicInitiatedBySelf()
{
    info_log("Business-Logic Called.");
}


int main(int argc, char** argv)
{
    SYSTEM_GLOBAL_INIT();

    start(&instaMsg, "920dfd80-2eef-11e5-b031-34689524378f", "ajaygarg456", NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf,
           "/home/ajay/subscriber");
}

