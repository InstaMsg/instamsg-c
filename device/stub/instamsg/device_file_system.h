#ifndef INSTAMSG_FILE_SYSTEM
#define INSTAMSG_FILE_SYSTEM

#include "../../../common/instamsg/driver/include/globals.h"

#if FILE_SYSTEM_ENABLED == 1

typedef struct FileSystem FileSystem;

struct FileSystem
{
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */
    char *fileName;

    int (*read) (FileSystem *fs, unsigned char* buffer, int len, unsigned char guaranteed);
    int (*write)(FileSystem *fs, unsigned char* buffer, int len);
    int (*copyFile)(FileSystem *fs, const char *oldPath, const char *newPath);
    int (*renameFile)(FileSystem *fs, const char *oldPath, const char *newPath);
    int (*deleteFile)(FileSystem *fs, const char *filePath);
    void (*getFileListing)(FileSystem *fs, char *buf, int maxValueLenAllowed, const char *directoryPath);
    long (*getFileSize)(FileSystem *fs, const char *filepath);
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */




    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
};
#endif

#endif
