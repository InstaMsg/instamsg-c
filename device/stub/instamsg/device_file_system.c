#include "./device_file_system.h"
#include "../../../common/instamsg/driver/include/globals.h"


#if FILE_SYSTEM_ENABLED == 1


/*
 * This method returns the file-listing in the directory specified, and populates the "buf".
 * The format is ::
 *
 *          {"publisher.sh":152,"subscriber.sh":140,"filetester.sh":137,"config.txt_filetester":148,"stdoutsub":56040,"build_ubuntu_14_04.sh":2892,"README.md":1502,"config.txt":128,"stdoutsub.c":6649,"config.txt_local_testing":128}
 */
void getFileListing(FileSystem *fs, char *buf, int maxValueLenAllowed, const char *directoryPath)
{
}


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

