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



#include "./include/globals.h"
#include "./include/instamsg.h"
#include "./include/httpclient.h"
#include "./include/sg_stdlib.h"
#include "./include/config.h"
#include "./include/upgrade.h"
#include "./include/sg_mem.h"

#if FILE_SYSTEM_ENABLED == 1
#include "./include/file_system.h"
#endif

#include "device_file_system.h"
#include <string.h>

#define HTTP_RESPONSE_STATUS_PREFIX "HTTP/"

HTTPResponse httpResponse;
static SG_Socket httpSocket;

static int getNextLine(SG_Socket *socket, char *buf, int *responseCode)
{
    while(1)
    {
        char ch[2] = {0};

        if(socket->read(socket, (unsigned char*)ch, 1, 1) == FAILURE) /* Pseudo-Blocking Call */
        {
            return FAILURE;
        }

        if(ch[0] == '\n')
        {
            if(strncmp(buf, HTTP_RESPONSE_STATUS_PREFIX, strlen(HTTP_RESPONSE_STATUS_PREFIX)) == 0)
            {
                char *firstToken = strtok(buf, " ");
                if(firstToken != NULL)
                {
                    char *secondToken = strtok(NULL, " ");
                    *responseCode = sg_atoi(secondToken);

                    if(*responseCode != HTTP_FILE_DOWNLOAD_SUCCESS)
                    {
                        sg_sprintf(LOG_GLOBAL_BUFFER, "%s%sResponse-Code is not %d, instead %d", FILE_UPLOAD, FILE_DOWNLOAD,
                                                                                                 HTTP_FILE_DOWNLOAD_SUCCESS, *responseCode);
                        error_log(LOG_GLOBAL_BUFFER);

                        return FAILURE;
                    }
                }
            }

            return SUCCESS;
        }
        else
        {
            if(ch[0] != '\r')
            {
                strcat(buf, ch);
            }
        }
    }

    return FAILURE;
}


static void generateRequest(const char *requestType,
                            const char *url,
                            KeyValuePairs *params,
                            KeyValuePairs *headers,
                            char *buf,
                            int maxLenAllowed,
                            unsigned char addFinalDelimiter)
{
    /*
     * Add the "GET" and "/1.txt"
     */
    sg_sprintf(buf, "%s %s", requestType, url);

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


static long getBytesIfContentLengthBytes(char *line)
{
    unsigned long numBytes = 0;

    char *headerKey = strtok(line, ":");
    char *headerValue = strtok(NULL, ":");

    if(headerKey && headerValue)
    {
        if(strcmp(headerKey, CONTENT_LENGTH) == 0)
        {
            numBytes = sg_atoi(headerValue);
        }
    }

    return numBytes;
}


#if FILE_SYSTEM_ENABLED == 1
/*
 * BYTE-LEVEL-REQUEST ::
 * ======================
 *
 * POST /api/beta/clients/00125580-e29a-11e4-ace1-bc764e102b63/files HTTP/1.0
 * Authorization: password
 * ClientId: 00125580-e29a-11e4-ace1-bc764e102b63
 * Content-Type: multipart/form-data; boundary=-----------ThIs_Is_tHe_bouNdaRY_78564$!@
 * Content-Length: 340
 *
 * -------------ThIs_Is_tHe_bouNdaRY_78564$!@
 *  Content-Disposition: form-data; name="file"; filename="filetester.sh"
 *  Content-Type: application/octet-stream
 *
 *  ./stdoutsub listener_topic --qos 2 --clientid 00125580-e29a-11e4-ace1-bc764e102b63 --password password --log /home/ajay/filetester --sub
 *
 *  -------------ThIs_Is_tHe_bouNdaRY_78564$!@--
 *
 *
 * BYTE-LEVEL-RESPONSE ::
 * =======================
 *
 * HTTP/1.0 200 OK
 * Content-Length: 89
 *
 * http://platform.instamsg.io:8081/files/1325d1f4-a585-4dbd-84e7-d4c6cfa6fd9d.filetester.sh
 */
void uploadFile(const char *url,
                const char *filename,
                KeyValuePairs *params,
                KeyValuePairs *headers,
                unsigned int timeout,
                HTTPResponse *httpResponse)
{

    int i = 0;
    unsigned int numBytes = 0;

    FileSystem fs;

    unsigned int totalLength;
    char *request, *secondLevel, *fourthLevel;

    request = (char *) sg_malloc(MAX_BUFFER_SIZE);
    secondLevel = (char *) sg_malloc(MAX_BUFFER_SIZE);
    fourthLevel = (char *) sg_malloc(MAX_BUFFER_SIZE);

    if((request == NULL) || (secondLevel == NULL) || (fourthLevel == NULL))
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFailure in memory allocation for uploadFile"), FILE_UPLOAD);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }
    memset(request, 0, MAX_BUFFER_SIZE);
    memset(secondLevel, 0, MAX_BUFFER_SIZE);
    memset(fourthLevel, 0, MAX_BUFFER_SIZE);

    httpSocket.socketCorrupted = 1;
	init_socket(&httpSocket, INSTAMSG_HTTP_HOST, INSTAMSG_HTTP_PORT, SOCKET_TCP, 1);
    if(httpSocket.socketCorrupted == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not initiate-socket for file-upload, not proceeding"), FILE_UPLOAD);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit2;
    }

    /* Now, generate the second-level (form) data
     * Please consult ::
     *
     *          http://stackoverflow.com/questions/8659808/how-does-http-file-upload-work
     */
    sg_sprintf(secondLevel, "--%s"                                                                   \
                         "\r\n"                                                                 \
                         "Content-Disposition: form-data; name=\"file\"; filename=\"%s\""       \
                         "\r\n"                                                                 \
                         "Content-Type: application/octet-stream"                               \
                         "\r\n\r\n", POST_BOUNDARY, filename);

    sg_sprintf(fourthLevel, "\r\n--%s--", POST_BOUNDARY);

    /*
     * Add the "Content-Length header
     */
    numBytes = instaMsg.singletonUtilityFs.getFileSize(&(instaMsg.singletonUtilityFs), filename);
    totalLength = strlen(secondLevel) + numBytes + strlen(fourthLevel);
    i = 0;

    {
        RESET_GLOBAL_BUFFER;

        while(1)
        {
            if(headers[i].key == NULL)
            {
                break;
            }

            if(strcmp(headers[i].key, CONTENT_LENGTH) == 0)
            {
                sg_sprintf((char*)GLOBAL_BUFFER, "%u", totalLength);
                headers[i].value = (char*)GLOBAL_BUFFER;
            }

            i++;
        }

        generateRequest("POST", url, params, headers, request, MAX_BUFFER_SIZE, 0);

        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFirst-stage URL that will be hit : [%s]"), FILE_UPLOAD, request);
        info_log(LOG_GLOBAL_BUFFER);
    }

    if(httpSocket.write(&httpSocket, (unsigned char*)request, strlen(request)) == FAILURE)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sError occurred while uploading POST data (FIRST LEVEL) for [%s]"), FILE_UPLOAD, filename);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }

    if(httpSocket.write(&httpSocket, (unsigned char*)secondLevel, strlen(secondLevel)) == FAILURE)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sError occurred while uploading POST data (SECOND LEVEL) for [%s]"), FILE_UPLOAD, filename);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }

    /*
     * Now, upload the actual file-data
     */
    init_file_system(&fs, (void *)filename);

    for(i = 0; i < numBytes; i++)
    {
        char ch[2] = {0};

        fs.read(&fs, (unsigned char*)ch, 1, 1);
        if(httpSocket.write(&httpSocket, (unsigned char*)ch, 1) == FAILURE)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sError occurred while uploading POST data (THIRD LEVEL) for [%s]"), FILE_UPLOAD, filename);
            error_log(LOG_GLOBAL_BUFFER);

            release_file_system(&fs);
            goto exit;
        }

        if((i % OTA_PING_BUFFER_SIZE) == 0)
        {
            /*
             * So that we do not time-out with the InstaMsg-Server.
             */
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%u / %u bytes uploaded ..."), i, numBytes);
            info_log(LOG_GLOBAL_BUFFER);

            sendPingReqToServer(&instaMsg);
            readAndProcessIncomingMQTTPacketsIfAny(&instaMsg);
        }
    }

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFile [%s] successfully uploaded worth [%u] bytes"), FILE_UPLOAD, filename, numBytes);
    info_log(LOG_GLOBAL_BUFFER);

    release_file_system(&fs);
    if(httpSocket.write(&httpSocket, (unsigned char*)fourthLevel, strlen(fourthLevel)) == FAILURE)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sError occurred while uploading POST data (FOURTH LEVEL) for [%s]"), FILE_UPLOAD, filename);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }


    numBytes = 0;
    while(1)
    {
        char beginPayloadDownload = 0;
        char *newLine;

        RESET_GLOBAL_BUFFER;

        newLine = (char*)GLOBAL_BUFFER;
        strcpy(newLine, "");

        if(getNextLine(&httpSocket, newLine, &(httpResponse->status)) == FAILURE)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sSocket error while reading URL-payload for uploaded file [%s] (stage 1)"),
                                          FILE_UPLOAD, filename);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

        /*
         * The actual payload begins after we receive an empty line.
         * Here, the payload contains the URL that needs to be passed to the peer.
         */
        if(strlen(newLine) == 0)
        {
            beginPayloadDownload = 1;
        }

        if(numBytes == 0)
        {
            numBytes = getBytesIfContentLengthBytes(newLine);
        }

        if(beginPayloadDownload == 1)
        {
            if(httpSocket.read(&httpSocket, (unsigned char*)(httpResponse->body), numBytes, 1) == FAILURE) /* Pseudo-Blocking Call */
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sSocket error while reading URL-payload for uploaded file [%s] (stage 2)"),
                                              FILE_UPLOAD, filename);
                error_log(LOG_GLOBAL_BUFFER);

                goto exit;
            }
            else
            {
                sg_sprintf(LOG_GLOBAL_BUFFER,
                           PROSTR("%sURL being provided to peer for uploaded file [%s] is [%s]"), FILE_UPLOAD, filename, httpResponse->body);
                info_log(LOG_GLOBAL_BUFFER);

                break;
            }
        }
    }

exit:
    release_socket(&httpSocket);

exit2:
    if(fourthLevel)
        sg_free(fourthLevel);

    if(secondLevel)
        sg_free(secondLevel);

    if(request)
        sg_free(request);


    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sHTTP-Response Status = [%d]"), FILE_UPLOAD, httpResponse->status);
    info_log(LOG_GLOBAL_BUFFER);

    return;
}
#endif


static void handleDownloadedBytesBatch(char *batch, int batchSize, int *totalBytesSoFar, int totalBytes)
{
    int j = 0;

    for(j = 0; j < batchSize; j++)
    {
        *totalBytesSoFar = *totalBytesSoFar + 1;
        copy_next_char(batch[j]);
    }

    if(((*totalBytesSoFar) % OTA_PING_BUFFER_SIZE) == 0)
    {
        /*
         * So that we do not time-out with the InstaMsg-Server.
        */
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%u / %u bytes downloaded ..."), *totalBytesSoFar, totalBytes);
        info_log(LOG_GLOBAL_BUFFER);

        sendPingReqToServer(&instaMsg);
        readAndProcessIncomingMQTTPacketsIfAny(&instaMsg);
    }
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
 * GET /files/1.txt HTTP/1.0\r\n\r\n
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
void downloadFile(const char *url,
                  const char *filename,
                  KeyValuePairs *params,
                  KeyValuePairs *headers,
                  unsigned int timeout,
                  HTTPResponse *httpResponse)
{
    unsigned int numBytes = 0;

    httpSocket.socketCorrupted = 1;
	init_socket(&httpSocket, INSTAMSG_HTTP_HOST, INSTAMSG_HTTP_PORT, SOCKET_TCP, 1);
    if(httpSocket.socketCorrupted == 1)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not initiate-socket for file-download, not proceeding"), FILE_DOWNLOAD);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit2;
    }

    {
        char *urlComplete;

        RESET_GLOBAL_BUFFER;
        urlComplete = (char*) GLOBAL_BUFFER;

        generateRequest("GET", url, params, headers, urlComplete, MAX_BUFFER_SIZE, 1);

        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sComplete URL that will be hit : [%s]"), FILE_DOWNLOAD, urlComplete);
        info_log(LOG_GLOBAL_BUFFER);

        /*
        * Fire the request-bytes over the socket-medium.
        */
        if(httpSocket.write(&httpSocket, (unsigned char*)urlComplete, strlen(urlComplete)) == FAILURE)
        {
            goto exit;
        }
    }

    numBytes = 0;
    while(1)
    {
        char beginPayloadDownload = 0;

        {
            char *newLine;

            RESET_GLOBAL_BUFFER;
            newLine = (char*)GLOBAL_BUFFER;

            strcpy(newLine, "");
            if(getNextLine(&httpSocket, newLine, &(httpResponse->status)) == FAILURE)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sError downloading file-metadata"), FILE_DOWNLOAD);
                info_log(LOG_GLOBAL_BUFFER);

                goto exit;
            }

            /*
            * The actual file-payload begins after we receive an empty line.
            */
            if(strlen(newLine) == 0)
            {
                beginPayloadDownload = 1;
            }

            if(numBytes == 0)
            {
                numBytes = getBytesIfContentLengthBytes(newLine);
            }
        }

        if(beginPayloadDownload == 1)
        {
            int i = 0, j = 0;

            prepare_for_new_binary_download();

            /* Now, we need to start reading the bytes */
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sBeginning downloading worth [%u] bytes"), FILE_DOWNLOAD, numBytes);
            info_log(LOG_GLOBAL_BUFFER);

            while(1)
            {
                int remaining_bytes = numBytes - i;
                if(remaining_bytes == 0)
                {
                    break;
                }

                if(remaining_bytes < OTA_BUFFER_SIZE)
                {
                    char ch[2] = {0};
                    for(j = i; j < numBytes; j++)
                    {
                        if(httpSocket.read(&httpSocket, (unsigned char*)ch, 1, 1) == FAILURE) /* Pseudo-Blocking Call */
                        {
                            tear_down_binary_download();
                            goto exit;
                        }

                        handleDownloadedBytesBatch(ch, 1, &i, numBytes);
                    }
                }
                else
                {
                    char ch[OTA_BUFFER_SIZE + 1] = {0};
                    if(httpSocket.read(&httpSocket, (unsigned char*)ch, OTA_BUFFER_SIZE, 1) == FAILURE) /* Pseudo-Blocking Call */
                    {
                        tear_down_binary_download();
                        goto exit;
                    }

                    handleDownloadedBytesBatch(ch, OTA_BUFFER_SIZE, &i, numBytes);
                }
            }

            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%u / %u bytes downloaded ..."), i, numBytes);
            info_log(LOG_GLOBAL_BUFFER);

            tear_down_binary_download();


            /*
             * Mark that file was downloaded successfully.
             */
            RESET_GLOBAL_BUFFER;
            generate_config_json((char*)GLOBAL_BUFFER, NEW_FILE_KEY, CONFIG_STRING, NEW_FILE_ARRIVED, "");
            save_config_value_on_persistent_storage(NEW_FILE_KEY, (char*)GLOBAL_BUFFER, 1);

            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFile-Download SUCCESS !!!!!!!!!!"), FILE_DOWNLOAD);
            info_log(LOG_GLOBAL_BUFFER);

exit:
            release_socket(&httpSocket);

exit2:
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sHTTP-Response Status = [%d]"), FILE_DOWNLOAD, httpResponse->status);
            info_log(LOG_GLOBAL_BUFFER);

            return;
        }
    }
}
