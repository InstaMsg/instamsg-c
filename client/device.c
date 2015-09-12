#include "src/common/include/instamsg.h"
#include "src/common/include/globals.h"

#include <string.h>

void coreLoopyBusinessLogicInitiatedBySelf()
{
    info_log("Business-Logic Called.");
}


int main(int argc, char** argv)
{
    globalSystemInit();
    start("2ebb9430-aa9d-11e4-a4c6-404014d5dd81", "ajaygarg789", NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, NULL);
}

