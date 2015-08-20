#include <string.h>
#include "../../../common/include/globals.h"

int main()
{
    char m[100];
    memset(m, 0, 100);

    sg_sprintf(m, "Test %s %d", "Ajay", 100);

    return 0;
}
