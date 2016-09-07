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




#include "device_file_system.h"

#if FILE_SYSTEM_ENABLED == 1

#include "./include/file_system.h"

void init_file_system(FileSystem *fs, void *arg)
{
    /* Register read-callback. */
	fs->read = file_system_read;

    /* Register write-callback. */
	fs->write = file_system_write;

    /* Register other-callbacks. */
    fs->copyFile = copyFile;
    fs->renameFile = renameFile;
    fs->deleteFile = deleteFile;
    fs->getFileListing = getFileListing;
    fs->getFileSize = getFileSize;

    fs->fileName = (char*)arg;
    connect_underlying_file_system_medium_guaranteed(fs);
}

void release_file_system(FileSystem *fs)
{
    release_underlying_file_system_medium_guaranteed(fs);
}

#endif
