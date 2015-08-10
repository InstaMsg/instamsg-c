#include "include/globals.h"
#include "include/instamsg.h"
#include "include/config.h"
#include "include/httpclient.h"

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


static void generateRequest(const char *requestType,
                            const char *url,
                            KeyValuePairs *params,
                            KeyValuePairs *headers,
                            unsigned char *buf,
                            int maxLenAllowed,
                            unsigned char addFinalDelimiter)
{
    /*
     * Add the "GET" and "/1.txt"
     */
    sprintf(buf, "%s %s", requestType, url);

    /*
     * Append the parameters (if any).
     */
    if(params != NULL)
    {
        int i = 0;
        while(1)
        {
            if(params[i].key == NULL)
            {
                break;
            }

            if(i == 0)
            {
                strcat(buf, "?");
            }
            else
            {
                strcat(buf, "&");
            }

            strcat(buf, params[i].key);
            strcat(buf, "=");
            strcat(buf, params[i].value);

            i++;
        }
    }

    /*
     * Add the "HTTP/1.0\r\n" part.
     */
    strcat(buf, " HTTP/1.0\r\n");

    /*
     * Add the headers (if any)
     */
    if(headers != NULL)
    {
        int i = 0;
        while(1)
        {
            if(headers[i].key == NULL)
            {
                break;
            }

            strcat(buf, headers[i].key);
            strcat(buf, ": ");
            strcat(buf, headers[i].value);
            strcat(buf, "\r\n");

            i++;
        }
    }

    /*
     * Finally, add the delimiter.
     */
    strcat(buf, "\r\n");
}



/*
 * Either of the URLs form work ::
 *
 *      http://platform.instamsg.io:8081/files/d2f9d9e7-e98b-4777-989e-605073a55efd.0003-Missed-a-path-export.patch
 *      /files/d2f9d9e7-e98b-4777-989e-605073a55efd.0003-Missed-a-path-export.patch
 */


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
HTTPResponse downloadFile(const char *url,
                          const char *filename,
                          KeyValuePairs *params,
                          KeyValuePairs *headers,
                          unsigned int timeout)
{
    int rc = FAILURE;
    Network network;
    HTTPResponse response;

    {
        NetworkParameters networkParametrs;
        readConfig(&config, "INSTAMSG_HTTP_HOST", STRING, &(networkParametrs.hostName));
        readConfig(&config, "INSTAMSG_HTTP_PORT", INTEGER, &(networkParametrs.port));

	    init_network(&network, &networkParametrs);
    }

    char request[MAX_BUFFER_SIZE] = {0};
    generateRequest("GET", url, params, headers, request, MAX_BUFFER_SIZE, 1);
    info_log(FILE_DOWNLOAD "Complete URL that will be hit : [%s]", request);

    /*
     * Fire the request-bytes over the network-medium.
     */
    if(network.write(&network, request, strlen(request)) == FAILURE)
    {
        terminateCurrentInstance = 1;
        goto exit;
    }

    long numBytes = 0;
    while(1)
    {
        char beginPayloadDownload = 0;

        char newLine[MAX_BUFFER_SIZE] = "";
        getNextLine(&network, newLine);

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
            if(strcmp(headerKey, CONTENT_LENGTH) == 0)
            {

                numBytes = atol(headerValue);
            }
        }

        if(beginPayloadDownload == 1)
        {
            char tempFileName[MAX_BUFFER_SIZE] = {0};
            sprintf(tempFileName, "~%s", filename);

            /*
             * Delete the file (it might have been downloaded partially some other time).
             */
            delete_file_system(tempFileName);

            FileSystem fs;
            init_file_system(&fs, (void *)tempFileName);

            // Now, we need to start reading the bytes
            info_log(FILE_DOWNLOAD "Beginning downloading of [%s] worth [%ld] bytes", tempFileName, numBytes);

            long i;
            for(i = 0; i < numBytes; i++)
            {
                char ch[2] = {0};

                if(network.read(&network, ch, 1) == FAILURE)
                {
                    terminateCurrentInstance = 1;
                    release_file_system(&fs);

                    goto exit;
                }

                fs.write(&fs, ch, 1);
            }

            release_file_system(&fs);

            /*
             * If we reach here, the file has been downloaded successfully.
             * So, move the "temp"-file to the actual file.
             */
            rename_file_system(tempFileName, filename);
            info_log(FILE_DOWNLOAD "File [%s] successfully moved to [%s] worth [%ld] bytes", tempFileName, filename, numBytes);

            rc = HTTP_FILE_DOWNLOAD_SUCCESS;

exit:
            release_network(&network);

            // TODO: Ideally, parse this 200 from the response.
            response.status = rc;
            return response;
        }
    }
}


HTTPResponse uploadFile(const char *url,
                        const char *filename,
                        KeyValuePairs *params,
                        KeyValuePairs *headers,
                        unsigned int timeout)
{

    /*
     * Check the headers contains a Content-Length field
     */
    int i = 0;
    long numBytes = 0;

    int rc = FAILURE;
    Network network;
    HTTPResponse response;

    {
        NetworkParameters networkParametrs;
        readConfig(&config, "INSTAMSG_HTTP_HOST", STRING, &(networkParametrs.hostName));
        readConfig(&config, "INSTAMSG_HTTP_PORT", INTEGER, &(networkParametrs.port));

	    init_network(&network, &networkParametrs);
    }


    char request[MAX_BUFFER_SIZE] = {0};


    /* Now, generate the isecond-level (form) data
     * Please consult ::
     *
     *          http://stackoverflow.com/questions/8659808/how-does-http-file-upload-work
     */
    char secondLevel[MAX_BUFFER_SIZE] = {0};
    sprintf(secondLevel, "--%s"                                                                   \
                         "\r\n"                                                                 \
                         "Content-Disposition: form-data; name=\"file\"; filename=\"%s\""       \
                         "\r\n"                                                                 \
                         "Content-Type: application/octet-stream"                               \
                         "\r\n\r\n", POST_BOUNDARY, filename);

    char fourthLevel[MAX_BUFFER_SIZE] = {0};
    sprintf(fourthLevel, "\r\n--%s--", POST_BOUNDARY);

    /*
     * Add the "Content-Length header
     */
    numBytes = instaMsg.systemUtils.getFileSize(&(instaMsg.systemUtils), filename);
    long totalLength = strlen(secondLevel) + numBytes + strlen(fourthLevel);
    i = 0;

    while(1)
    {
        if(headers[i].key == NULL)
        {
            break;
        }

        if(strcmp(headers[i].key, CONTENT_LENGTH) == 0)
        {
            char value[MAX_BUFFER_SIZE] = {0};
            sprintf(value, "%ld", totalLength);

            headers[i].value = value;
        }

        i++;
    }

    generateRequest("POST", url, params, headers, request, MAX_BUFFER_SIZE, 0);
    info_log(FILE_UPLOAD "First-stage URL that will be hit : [%s]", request);

    if(network.write(&network, request, strlen(request)) == FAILURE)
    {
        error_log(FILE_UPLOAD "Error occurred while uploading POST data (FIRST LEVEL) for [%s]", filename);
        terminateCurrentInstance = 1;

        goto exit;
    }

    if(network.write(&network, secondLevel, strlen(secondLevel)) == FAILURE)
    {
        error_log(FILE_UPLOAD "Error occurred while uploading POST data (SECOND LEVEL) for [%s]", filename);
        terminateCurrentInstance = 1;

        goto exit;
    }

    /*
     * Now, upload the actual file-data
     */
    FileSystem fs;
    init_file_system(&fs, (void *)filename);

    for(i = 0; i < numBytes; i++)
    {
        char ch[2] = {0};

        fs.read(&fs, ch, 1);
        if(network.write(&network, ch, 1) == FAILURE)
        {
            error_log(FILE_UPLOAD "Error occurred while uploading file-byte(s) for [%s]", filename);
            terminateCurrentInstance = 1;

            release_file_system(&fs);
            goto exit;
        }
    }

    info_log(FILE_UPLOAD "File [%s] successfully uploaded worth [%ld] bytes", filename, numBytes);

    release_file_system(&fs);
    if(network.write(&network, fourthLevel, strlen(fourthLevel)) == FAILURE)
    {
        error_log(FILE_UPLOAD "Error occurred while uploading POST data (FOURTH LEVEL) for [%s]", filename);
        terminateCurrentInstance = 1;

        goto exit;
    }


    unsigned char readLast = 0;
    numBytes = 0;
    while(1)
    {
        char beginPayloadDownload = 0;

        char newLine[MAX_BUFFER_SIZE] = "";
        getNextLine(&network, newLine);

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
             * url-value to be downloaded.
             */
            if(strcmp(headerKey, CONTENT_LENGTH) == 0)
            {

                numBytes = atol(headerValue);
            }
        }

        if(beginPayloadDownload == 1)
        {
            network.read(&network, response.body, numBytes);
            debug_log(FILE_UPLOAD "URL being provided to peer for uploaded file [%s] is [%s]", filename, response.body);

            break;
        }
    }

    rc = HTTP_FILE_UPLOAD_SUCCESS;

exit:
    release_network(&network);

    response.status = rc;
    return response;
}
