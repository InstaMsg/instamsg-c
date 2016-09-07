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



#ifndef INSTAMSG_LOGGER
#define INSTAMSG_LOGGER

#include "./serial_logger.h"
#include "./globals.h"

#include "device_file_system.h"

#if FILE_SYSTEM_ENABLED == 1
typedef struct FileLogger FileLogger;
struct FileLogger
{
    FileSystem fs;
};

void init_file_logger(FileLogger *fileLogger, void *arg);
void release_file_logger(FileLogger *fileLogger);
#endif

#if FILE_LOGGING_ENABLED == 1
extern FileLogger fileLogger;
#endif


#define INSTAMSG_LOG_LEVEL_DISABLED 0
#define INSTAMSG_LOG_LEVEL_INFO     1
#define INSTAMSG_LOG_LEVEL_ERROR    2
#define INSTAMSG_LOG_LEVEL_DEBUG    3

typedef int (*LOG_WRITE_FUNC)(void *logger_medium, unsigned char *buffer, int len);
extern int currentLogLevel;



/*
 *********************************************************************************************************************
 **************************************** PUBLIC APIs *****************************************************************
 **********************************************************************************************************************
 */

/*
 * Logging at INFO level.
 */
void info_log(char *log);

/*
 * Logging at ERROR level.
 */
void error_log(char *log);

/*
 * Logging at DEBUG level.
 */
void debug_log(char *log);



#endif
