/*****************************************************************************
*
*  exosite.c - Exosite cloud communications.
*  Copyright (C) 2012 Exosite LLC
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Exosite LLC nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/
#include "exosite_pal.h"
#include "exosite.h"


static const char STR_CIK_HEADER[] = "X-Exosite-CIK: ";
static const char STR_CONTENT_LENGTH[] = "Content-Length: ";
static const char STR_READ_URL[] = "GET /onep:v1/stack/alias?";
static const char STR_WRITE_URL[] = "POST /onep:v1/stack/alias ";
static const char STR_ACTIVATE_URL[] = "POST /provision/activate ";
static const char STR_HTTP[] = "HTTP/1.1";
static const char STR_HOST[] = "Host: m2.exosite.com";
static const char STR_ACCEPT[] = "\r\nAccept: application/x-www-form-urlencoded; charset=utf-8";
static const char STR_CONTENT[] = "Content-Type: application/x-www-form-urlencoded; charset=utf-8";
static const char STR_CRLF[] = "\r\n";

// local functions
//static int32_t init_UUID();
//static int32_t readResponse(int32_t socket, char * expectedCode);
static uint8_t exosite_connect();
static uint8_t exosite_disconnect();
//static void sendLine(int32_t socket, unsigned char LINE, const char * payload);
static uint8_t exosite_checkResponse(char * response, const char * code);
static uint8_t exosite_readResponse(char * response, uint16_t responseSize);

static uint16_t exosite_socketRead( char * buf, uint16_t len, uint16_t * responseSize);
static uint16_t exosite_socketWrite( char * buf, uint16_t len);

#define STR_VENDOR  "vendor="
#define STR_MODEL   "&model="
#define STR_SN      "&sn="

static char cikBuffer[CIK_LENGTH];
static char vendorBuffer[MAX_VENDOR_LENGTH];
static char modelBuffer[MAX_MODEL_LENGTH];
static char uuidBuffer[MAX_UUID_LENGTH];

static char rxBuffer[RX_BUFFER_SIZE];

/*!
 * Used to determine if there is currently an open socket.  This value is 1 if 
 * we have an open socket, else 0.
 */
static uint8_t isSocketOpen = 0;


static EXO_STATE initState = EXO_STATE_NOT_COMPLETE;

/*!
 * \brief Reset the cik to ""
 *
 * 
 *
 * \return Returns 0 if successful, else error code
 * \sa
 * \note
 * \warning
 */
uint8_t exosite_resetCik()
{
    exoPal_setCik("");
    return 0;
}

/*!
 * \brief  Initializes the Exosite libraries and attempts to activate the
 *          with Exosite
 *
 * This **MUST** be called before any other exosite library calls are called. 
 *
 * Assumes that the modem is setup and ready to make a socket connection.
 * This will fail if activation fails.  After initialization, this function 
 * calls exosite_activate
 *
 * \param[in] vendor Pointer to string containing vendor name
 * \param[in] model Pointer to string containing model name
 *
 * \return Device Activation status
 */
EXO_STATE exosite_init(const char * vendor, const char *model)
{
    // reset state
    initState = EXO_STATE_NOT_COMPLETE;
    uint8_t retStatus = 0;
    // get cik and uuid and any other nvm stored data, into ram.
    retStatus |= exoPal_getCik(cikBuffer);
    retStatus |= exoPal_getUuid(uuidBuffer);
    exoPal_memcpy(vendorBuffer, vendor, MAX_VENDOR_LENGTH);
    exoPal_memcpy(modelBuffer, model, MAX_MODEL_LENGTH);
    // create activation request
    if (retStatus)
    {
        initState = EXO_STATE_INIT_ERROR;
        return initState;
    }
    initState = exosite_activate();
    if (initState == EXO_STATE_VALID_CIK)
    {
        initState = EXO_STATE_INIT_COMPLETE;
    }
    return initState;
}




/*!
 * \brief  Makes a provisioning request to Exosite.
 *
 * 
 *
 * \return The devices activation status
 */
EXO_STATE exosite_activate()
{

    // Try and activate device with Exosite, four possible cases:
    // * We don't have a stored CIK and receive a 200 response with a CIK
    //    * Means device was enabled and this was our first connection
    // * We don't have a stored CIK and receive a 409 response
    //    * The device is not enabled.
    // * We have a stored CIK and receive a 409 response.
    //     *  Device has already been activated and has a valid CIK
    // * We have a stored CIK and receive a 401 response
    //    * R/W error

    uint8_t vendorLength = exoPal_strlen(vendorBuffer);
    uint8_t modelLength = exoPal_strlen(modelBuffer);
    uint8_t uuidLength = exoPal_strlen(uuidBuffer);

    // get body length
    uint16_t bodyLength =   sizeof(STR_VENDOR) - 1 + 
                            sizeof(STR_MODEL) - 1 + 
                            sizeof(STR_SN) - 1;
    bodyLength += vendorLength + modelLength + uuidLength;

    // assume content length will never be over 9999 bytes
    char contentLengthStr[5];
    uint8_t len_of_contentLengthStr = exoPal_itoa((int)bodyLength, contentLengthStr, 5);
 

    exosite_connect();


    // send request
    exoPal_socketWrite(STR_ACTIVATE_URL, sizeof(STR_ACTIVATE_URL) - 1);
    exoPal_socketWrite(STR_HTTP, sizeof(STR_HTTP) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send Host header
    exoPal_socketWrite(STR_HOST, sizeof(STR_HOST) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

        // send content type header
    exoPal_socketWrite(STR_CONTENT, sizeof(STR_CONTENT) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send content length header
    exoPal_socketWrite(STR_CONTENT_LENGTH, sizeof(STR_CONTENT_LENGTH) - 1);
    exoPal_socketWrite(contentLengthStr, len_of_contentLengthStr);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send body
    exoPal_socketWrite(STR_VENDOR, sizeof(STR_VENDOR) - 1);
    exoPal_socketWrite(vendorBuffer, vendorLength);
    exoPal_socketWrite(STR_MODEL, sizeof(STR_MODEL) - 1);
    exoPal_socketWrite(modelBuffer, modelLength);
    exoPal_socketWrite(STR_SN, sizeof(STR_SN) - 1);
    exoPal_socketWrite(uuidBuffer, uuidLength);


    exosite_disconnect();
    EXO_STATE retVal = EXO_STATE_CONNECTION_ERROR;
    char cik[] = {"123123123123"};

    uint16_t responseLen;
    
    exoPal_socketRead( rxBuffer, RX_BUFFER_SIZE, &responseLen);
    
    

    if (exosite_checkResponse(rxBuffer, "200"))
    {
        // we received a CIK.

        //find first '\n' char from end of response
        for (int i = responseLen; i > 0; i--)
        {
            if (rxBuffer[i] == '\n')
            {
                // check that we're where we think we should be.
                if ((responseLen-i - 1) != CIK_LENGTH)
                {
                    // the data after the '\n' did not equal the length
                    // of the CIK.  An error of some sorts occurred
                    retVal = EXO_STATE_CONNECTION_ERROR;
                    // end this loop and return our error
                    i = 0;
                }
                else
                {
                    // copy cik into mem.
                    exoPal_setCik(&rxBuffer[i + 1]);
                    retVal = EXO_STATE_VALID_CIK;
                    i = 0;
                }
            }
        }
    }
    else if (exosite_checkResponse(rxBuffer, "409"))
    {
        // TODO: validate the cik instead of checking the first char for '\0'
        if (cik[0] == '\0')
        {
            // if we don't have a CIK in nvm and we receive a 409
            // The device isn't enabled in the dashboard
            retVal = EXO_STATE_DEVICE_NOT_ENABLED;
        }
        else
        {
            // If we receive a 409 and we do have a valid CIK, we will
            // assume we are good to go.
            retVal = EXO_STATE_VALID_CIK;
        }
    }
    else if (exosite_checkResponse(rxBuffer, "401"))
    {
        // RW error
        retVal = EXO_STATE_R_W_ERROR;
    }

    return retVal;

}


/*!
 * \brief Checks if the given cik is valid
 *
 * Checks for atleast 40 valid hexidecimal bytes.
 *
 * \param[in] cik array of CIK_LENGTH bytes
 *
 * \return Returns 1 if successful, else 0
 * \sa
 * \note
 * \warning
 */
uint8_t exosite_isCIKValid(char cik[CIK_LENGTH])
{
    uint8_t i;

    for (i = 0; i < CIK_LENGTH; i++)
    {
        if (!(cik[i] >= 'a' && cik[i] <= 'f' || cik[i] >= '0' && cik[i] <= '9'))
        {
            return 0;
        }
    }

    return 1;
}



/*!
 *  \brief  Programs a new CIK to flash / non volatile
 *
 * \param[in] pCIK Pointer to CIK
 *
 */
void exosite_setCIK(char * pCIK)
{
    exoPal_setCik((char *)pCIK);

    return;
}



/*!
 *  \brief  Retrieves a the CIK from NVM and places it in to the string pointed
 *			at by \a cik
 *
 * \param[out] cik Pointer to CIK
 *
 */
void exosite_getCIK(char * cik)
{
    exoPal_getCik((char *)cik);

    return;
}


/*!
 *  \brief  writes data to Exosite
 *
 * \param[in] writeData Pointer to buffer of data to write to Exosite
 * \param[in] length length of data in buffer
 *
 * \return TODO
 *
 */
uint8_t exosite_write(const char * writeData, uint16_t length)
{

    // connect to exosite
    uint8_t connection_status = exosite_connect();

    // return error message if connect failed.
    if (connection_status != 0)
    {
        return connection_status;
    }

    // assume content length won't be greater than 9999.
    char * contentLengthStr[5];

    uint8_t len_of_contentLengthStr = exoPal_itoa((int)length, *contentLengthStr, 5);

    // send request
    exoPal_socketWrite(STR_WRITE_URL, sizeof(STR_WRITE_URL) - 1);
    exoPal_socketWrite(STR_HTTP, sizeof(STR_HTTP) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send Host header
    exoPal_socketWrite(STR_HOST, sizeof(STR_HOST) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);
    
    // send cik header
    exoPal_socketWrite(STR_CIK_HEADER, sizeof(STR_CIK_HEADER) - 1);
    exoPal_socketWrite(cikBuffer, sizeof(cikBuffer) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send content type header
    exoPal_socketWrite(STR_CONTENT, sizeof(STR_CONTENT) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send content length header
    exoPal_socketWrite(STR_CONTENT_LENGTH, sizeof(STR_CONTENT_LENGTH) - 1);
    exoPal_socketWrite(*contentLengthStr, len_of_contentLengthStr);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send body
    exoPal_socketWrite(writeData, length);

    uint16_t responseLength = 0;
    // get response
    exoPal_socketRead(rxBuffer, RX_BUFFER_SIZE, &responseLength);


    exosite_disconnect();

    // 204 "No content"
    if (exosite_checkResponse(rxBuffer, "204"))
    {
       return 0;
    }
    else
    {
        return 1;
    }
}



/*!
 *  \brief  Reads data from Exosite
 *
 * \param[in] alias Name of data source alias to read from
 * \param[out] readResponse buffer to place read respone in
 * \param[in] buflen length of buffer
 * \param[out] length data placed into readResponse
 *
 * \return Errorcode if fail, else 0
 *
 */
uint8_t exosite_read(const char * alias, char * readResponse, uint16_t buflen, uint16_t * length)
{
    // connect to exosite
    uint8_t connection_status = exosite_connect();

    // return error message if connect failed.
    if (connection_status != 0)
    {
        return connection_status;
    }

    // send request
    exoPal_socketWrite(STR_READ_URL, sizeof(STR_READ_URL) - 1);
    exoPal_socketWrite(alias, exoPal_strlen(alias));
    exoPal_socketWrite(STR_HTTP, sizeof(STR_HTTP) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send Host header
    exoPal_socketWrite(STR_HOST, sizeof(STR_HOST) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send cik header
    exoPal_socketWrite(STR_CIK_HEADER, sizeof(STR_CIK_HEADER) - 1);
    exoPal_socketWrite(cikBuffer, sizeof(cikBuffer) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send content type header
    exoPal_socketWrite(STR_CONTENT, sizeof(STR_CONTENT) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);

    // send accept header
    exoPal_socketWrite(STR_ACCEPT, sizeof(STR_ACCEPT) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);
    exoPal_socketWrite(STR_CRLF, sizeof(STR_CRLF) - 1);
    

    uint16_t responseLength = 0;
    // get response
    exosite_socketRead(rxBuffer, RX_BUFFER_SIZE, &responseLength);


    exosite_disconnect();

    // 204 "No content"
    if (exosite_checkResponse(rxBuffer, "200"))
    {
        //find first '\n' char from end of response
        for (int i = responseLength; i > 0; i--)
        {
            // find last \n
            if (rxBuffer[i] == '\n')
            {
                uint8_t charNotMatch = 0;
                for (uint16_t j = 1; (j <= i) && i > 0; j++)
                {
                    // If we're at the end of the inputted string?
                    if (alias[j-1] == '\0')
                    {
                        // if all chars match, we found the key
                        if (!charNotMatch)
                        {
                            // move j passed the '='
                            j++;

                            for (uint16_t k = 0;
                                 (k <= buflen) && ((i + j + k) <= responseLength);
                                 k++)
                            {
                                // copy remaining data into buffer
                                readResponse[k] = rxBuffer[i+j+k];
                                *length = k;
                            }
                            i = 0;
                        }
                        else
                        {
                            // match not found, exit
                            i = 0;
                            *length = 0;
                        }
                    }

                    // confirm letter by letter
                    charNotMatch |= !(rxBuffer[i+j] == alias[j-1]);
                }
            }
        }
    }

    return 0;
}







/*!
 * \brief Connects to Exosite
 *
 * This would typically be a call to open a socket 
 *
 * \return Returns a 0 if socket was successfully opened, else returns the
 *          error code
 * \sa
 * \note
 * \warning
 */
uint8_t exosite_connect(void)
{
    // open socket to exosite
    
    return exoPal_tcpSocketOpen();
}

/*!
 * \brief Connects to Exosite
 *
 * This would typically be a call to close a socket 
 *
 * \return Returns a 0 if socket was successfully close, else returns the
 *          error code
 * \sa
 * \note
 * \warning
 */
uint8_t exosite_disconnect(void)
{
    // open socket to exosite
    return 0;
}


/*!
 * \brief  determines if response matches code
 *
 * \param[in] code Pointer to expected HTTP response code
 * \param[in] response an HTTP response string
 *
 * \return 1 if match, 0 if no match
 *
 */
uint8_t exosite_checkResponse(char * response, const char * code)
{
    if (code[0] == response[9] && code[1] == response[10] && code[2] == response[11])
    {
        return 1;
    }

    return 0;
}



/*!
 * \brief  Retrieves data from a socket
 *
 * \note This library assumes that the response buffer will alway be large 
 *          enough to handle the response string.
 *
 * \param[in] buf Buffer to place received data into.
 * \param[in] len Length of buf
 *
 * \return length of response
 *
 */
static uint16_t exosite_socketRead( char * buf, uint16_t len, uint16_t * responseSize)
{
    uint16_t rxLen = 0;
    
    exoPal_socketRead( buf, len, &rxLen);

    return rxLen;
}

/*!
 * \brief  Writes data to a socket
 *
 * \param[in] buf Data to write to socket
 * \param[in] len Length of buf
 *
 * \return length of response
 *
 */
uint16_t exosite_socketWrite( char * buf, uint16_t len)
{
    uint16_t rxLen = 0;

    rxLen = exoPal_socketWrite( buf, len);

    return rxLen;
}



