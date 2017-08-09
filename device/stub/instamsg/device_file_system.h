#ifndef INSTAMSG_FILE_SYSTEM
#define INSTAMSG_FILE_SYSTEM

#include "../../../common/instamsg/driver/include/globals.h"

#if FILE_SYSTEM_ENABLED == 1

typedef struct FileSystem FileSystem;

struct FileSystem
{
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */
    char *fileName;

    void (*getFileListing)(FileSystem *fs, char *buf, int maxValueLenAllowed, const char *directoryPath);
    long (*getFileSize)(FileSystem *fs, const char *filepath);
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */




    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
};
#endif

#endif
