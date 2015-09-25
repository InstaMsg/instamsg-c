#ifdef FILE_SYSTEM_INTERFACE_ENABLED

#ifndef INSTAMSG_FILE_SYSTEM
#define INSTAMSG_FILE_SYSTEM

#include <stdio.h>

typedef struct FileSystem FileSystem;

struct FileSystem
{
    char *fileName;

    /*
     * Any extra fields may be added here.
     */
    FILE *fp;

    int (*read) (FileSystem *fs, unsigned char* buffer, int len, unsigned char guaranteed);
    int (*write)(FileSystem *fs, unsigned char* buffer, int len);
    int (*renameFile)(FileSystem *fs, const char *oldPath, const char *newPath);
    int (*deleteFile)(FileSystem *fs, const char *filePath);
    void (*getFileListing)(FileSystem *fs, char *buf, int maxValueLenAllowed, const char *directoryPath);
    long (*getFileSize)(FileSystem *fs, const char *filepath);
};
#endif

#endif
