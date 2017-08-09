/*******************************************************************************
 *
 * Copyright (c) 2014 SenseGrow, Inc.
 *
 * SenseGrow Internet of Things (IoT) Client Frameworks
 * http://www.sensegrow.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Contributors:
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/



#ifndef INSTAMSG_FILE_SYSTEM_COMMON
#define INSTAMSG_FILE_SYSTEM_COMMON

#include "device_file_system.h"

#if FILE_SYSTEM_ENABLED == 1
/*
 * Global-functions callable.
 */
void init_file_system(FileSystem *fs, void *arg);
int sg_file_read(FileSystem *fs, unsigned char* buffer, int len, unsigned char guaranteed);
int sg_file_write(FileSystem *fs, unsigned char* buffer, int len);
int sg_copyFile(FileSystem *fs, const char *oldPath, const char *newPath);
int sg_renameFile(FileSystem *fs, const char *oldPath, const char *newPath);
void release_file_system(FileSystem *fs);

void sg_readLine(FILE_STRUCT *fp, char *buffer, int maxBufferLength);
void sg_readFile(const char *filePath, char *buffer, int maxBufferLength);
void sg_writeFile(const char *filePath, char *buffer);
int sg_appendLine(const char *filePath, const char *buffer);
int sg_createEmptyFile(const char *filePath);



/*
 * Must not be called directly.
 * Instead must be called as ::
 *
 *      fs->getFileListing
 *      fs->getFileSize
 */
void getFileListing(FileSystem *fs, char *buf, int maxValueLenAllowed, const char *directoryPath);
long getFileSize(FileSystem *fs, const char *filepath);
void getNextFileToProcessPath(char *folder, char *path, int maxBufferLength);


#endif

#endif
