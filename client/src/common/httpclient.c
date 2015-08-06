#include "include/globals.h"
#include "include/instamsg.h"

#include "include/instamsg_vendor_common.h"
#include "instamsg_vendor.h"

#include <string.h>


static void getNextLine(Network *network, unsigned char *buf)
{
    while(1)
    {
        char ch[2] = {0};

        if(network->read(network, ch, 1) == FAILURE)
        {
            terminateCurrentInstance = 1;
            return;
        }

        if(ch[0] == '\n')
        {
            return;
        }
        else
        {
            if(ch[0] != '\r')
            {
                strcat(buf, ch);
            }
        }
    }
}


/*
 * BYTE-LEVEL-REQUEST ::
 * ======================
 *
 * GET /1.txt HTTP/1.0\r\n\r\n
 *
 *
 * BYTE-LEVEL-RESPONSE ::
 * =======================
 *
 * HTTP/1.1 200 OK
 * Date: Wed, 05 Aug 2015 09:43:26 GMT
 * Server: Apache/2.4.7 (Ubuntu)
 * Last-Modified: Wed, 05 Aug 2015 09:14:51 GMT
 * ETag: "f-51c8cd5d313d7"
 * Accept-Ranges: bytes
 * Content-Length: 15
 * Connection: close
 * Content-Type: text/plain
 *
 * echo "hi ajay"
*/
int downloadFile(Network *network, const char *url, const char *downloadedFileName, unsigned int timeout)
{
    char request[MAX_BUFFER_SIZE] = {0};
    sprintf(request, "GET %s HTTP/1.0\r\n\r\n", url);

    /*
     * Fire the request-bytes over the network-medium.
     */
    if(network->write(network, request, strlen(request)) == FAILURE)
    {
        terminateCurrentInstance = 1;
        return;
    }


    while(1)
    {
        long numBytes = 0;
        char beginPayloadDownload = 0;

        char newLine[MAX_BUFFER_SIZE] = "";
        getNextLine(network, newLine);

        /*
         * The actual file-payload begins after we receive an empty line.
         *
         * We need to track this here itself, because later usage of "strtok_r"
         * (to parse the tokens) destroys/modifies this string.
         */
        if(strlen(newLine) == 0)
        {
            beginPayloadDownload = 1;
        }

        char *saveptr;
        char *headerKey = strtok_r(newLine, ":", &saveptr);
        char *headerValue = strtok_r(NULL, ":", &saveptr);

        if(headerKey && headerValue)
        {
            /*
             * After we have got the "Content-Length" header, we know the size of the
             * file-payload to be downloaded.
             */
            if(strcmp(headerKey, "Content-Length") == 0)
            {
                numBytes = atol(headerValue);
            }
        }

        if(beginPayloadDownload == 1)
        {
            char tempFileName[MAX_BUFFER_SIZE] = {0};
            sprintf(tempFileName, "~%s", downloadedFileName);

            /*
             * Delete the file (it might have been downloaded partially some other time).
             */
            delete_file_system(tempFileName);

            FileSystem fs;
            init_file_system(&fs, (void *)tempFileName);

            // Now, we need to start reading the bytes
            long i;
            for(i = 0; i < numBytes; i++)
            {
                char ch[2] = {0};

                if(network->read(network, ch, 1) == FAILURE)
                {
                    terminateCurrentInstance = 1;
                    release_file_system(&fs);

                    return FAILURE;
                }

                fs.write(&fs, ch, 1);
            }

            release_file_system(&fs);

            /*
             * If we reach here, the file has been downloaded successfully.
             * So, move the "temp"-file to the actual file.
             */
            rename_file_system(tempFileName, downloadedFileName);
            info_log("File [%s] successfully moved to [%s]", tempFileName, downloadedFileName);

            // TODO: Ideally, parse this 200 from the response.
            return HTTP_FILE_DOWNLOAD_SUCCESS;
        }
    }
}
