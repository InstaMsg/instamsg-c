#include "../../instamsg/driver/include/instamsg.h"

static void coreLoopyBusinessLogicInitiatedBySelf()
{
}

int main(int argc, char** argv)
{
    start(NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 1);
}
