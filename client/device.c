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
    start("00125580-e29a-11e4-ace1-bc764e102b63", "ajaygarg789", NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, NULL);
}

