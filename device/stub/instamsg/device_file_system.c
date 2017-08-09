#include "./device_file_system.h"
#include "../../../common/instamsg/driver/include/globals.h"


#if FILE_SYSTEM_ENABLED == 1

/*
 * This method returns a long-value, specifying the size of file in bytes.
 */
long getFileSize(FileSystem *fs, const char *filepath)
{
#error "Function not implemented."
}


/*
 * Gets the first available file-path in the folder
 */
void getNextFileToProcessPath(char *folder, char *path, int maxBufferLength)
{
}

#endif

