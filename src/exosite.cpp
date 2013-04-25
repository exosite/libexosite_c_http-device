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
#include "exosite_hal.h"
#include "exosite_meta.h"
#include "exosite.h"

#include <string.h>


//local defines
static char exosite_provision_info[50];


enum lineTypes
{
    CIK_LINE,		/*!< line contains CIK */
    HOST_LINE,
    CONTENT_LINE,
    ACCEPT_LINE,
    LENGTH_LINE,
    GETDATA_LINE,
    POSTDATA_LINE,
    VENDOR_LINE,
    EMPTY_LINE
};

#define STR_CIK_HEADER "X-Exosite-CIK: "
#define STR_CONTENT_LENGTH "Content-Length: "
#define STR_GET_URL "GET /onep:v1/stack/alias?"
#define STR_HTTP " HTTP/1.1\r\n"
#define STR_HOST "Host: m2.exosite.com\r\n"
#define STR_ACCEPT "Accept: application/x-www-form-urlencoded; charset=utf-8\r\n"
#define STR_CONTENT "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n"
#define STR_VENDOR "vendor="
#define STR_MODEL "model="
#define STR_SN "sn="
#define STR_CRLF "\r\n"

// local functions
static int32_t info_assemble(const char * vendor, const char *model);
static int32_t init_UUID();
static void update_m2ip(void);
static int32_t readResponse(int32_t socket, char * expectedCode);
static int32_t connect_to_exosite();
static void sendLine(int32_t socket, unsigned char LINE, const char * payload);
static int32_t checkResponse(char * response, char * code);
static uint16_t getResponse(int32_t socket, char * buf, uint16_t len);

// global functions


// externs
extern char *itoa(int n, char *s, int b);

// global variables

/*!
* Used to track how many times a write to the Exosite platform has failed
*/
static unsigned char exositeWriteFailures = 0;
int32_t cloud_status = -1;



/*!
* \brief  The function initializes the custom's vendor and model name for
*          provision prepare
*
* [detailed description]
*
* \param[in] vendor Pointer to string containing vendor name
* \param[in] model Pointer to string containing model name
*
* \return length of assembled customize's vendor information
*/
int32_t info_assemble(const char * vendor, const char *model)
{
    char vendor_info[95];
    int32_t info_len = 0;
    int32_t assemble_len = 0;

    // verify the assembly length
    assemble_len = strlen(STR_VENDOR) + strlen(vendor)
                   + strlen(STR_MODEL) + strlen(model)
                   + strlen(STR_SN) + 3;
    if (assemble_len > 95)
        return info_len;

    // vendor=
    memcpy(vendor_info, STR_VENDOR, strlen(STR_VENDOR));
    info_len = strlen(STR_VENDOR);

    // vendor="custom's vendor"
    memcpy(&vendor_info[info_len], vendor, strlen(vendor));
    info_len += strlen(vendor);

    // vendor="custom's vendor"&
    vendor_info[info_len] = '&'; // &
    info_len += 1;

    // vendor="custom's vendor"&model=
    memcpy(&vendor_info[info_len], STR_MODEL, strlen(STR_MODEL));
    info_len += strlen(STR_MODEL);

    // vendor="custom's vendor"&model="custom's model"
    memcpy(&vendor_info[info_len], model, strlen(model));
    info_len += strlen(model);

    // vendor="custom's vendor"&model="custom's model"&
    vendor_info[info_len] = '&'; // &
    info_len += 1;

    // vendor="custom's vendor"&model="custom's model"&sn=
    memcpy(&vendor_info[info_len], STR_SN, strlen(STR_SN));
    info_len += strlen(STR_SN);

    memcpy(exosite_provision_info, vendor_info, info_len);

    return info_len;
}



/*! 
* \brief  Feedback the exosite cloud status
*
* \return 1 success; 0 failure
*/
int32_t Exosite_Cloud_Status(void)
{
    return cloud_status;
}



/*!
* \brief  The function initializes the exosite meta structure ,UUID and
*          provision information
*
* \param[in] vendor Pointer to string containing vendor name
* \param[in] model Pointer to string containing model name
*
* \return length of assembled customize's vendor information
*/
void Exosite_Init(const char *vendor, const char *model)
{

    exoHAL_initModem();

    info_assemble(vendor, model);
    exosite_meta_init();        //always initialize our meta structure
    init_UUID();          //always check to see if the UUID is up to date

    //setup some of our globals for operation
    exositeWriteFailures = 0;
    
    EXOSITE_DEVICE_ACTIVATION_STATE stat = activate_device();
    
    if (stat != VALID_CIK)
    {
        if (stat == CONNECTION_ERROR)
        {
            exoHAL_ShowErrorMessage("Conn err", sizeof("Conn err"));
        }
        else if (stat == DEVICE_NOT_ENABLED)
        {
            exoHAL_ShowErrorMessage("Dev Not Enab", sizeof("Dev Not Enab"));
        }
        else if (stat == R_W_ERROR)
        {
            exoHAL_ShowErrorMessage("R_W Error", sizeof("R_W Error"));
        }
        else
        {
            exoHAL_ShowErrorMessage("Unknown: 1", sizeof("Unknown: 1"));
        }
        while(1);
    }
    
    return;
}


/*!
*  \brief  Called after Init has been ran in the past, but maybe comms were
*          down and we have to keep trying...
*
*/
void Exosite_Cloud_Setup(void)
{
    update_m2ip();      //check our IP api to see if the old IP is advertising a new one
    activate_device();  //the moment of truth - can this device provision with the Exosite cloud?...
    cloud_status = Exosite_CheckCIK();

    return;
}


/*****************************************************************************
*
* Exosite_CheckCIK
*
*  \param  None
*
*  \return 1 - CIK was valid, 0 - CIK was invalid.
*
*  \brief  Validate the CIK
*
*****************************************************************************/
/*!
*  \brief  Reads data from Exosite
*
*
* \return 1 - CIK was valid, 0 - CIK was invalid.
*
*/
int32_t Exosite_CheckCIK(void)
{
    unsigned char i;
    char tempCIK[CIK_LENGTH];

    Exosite_GetCIK(tempCIK);

    for (i = 0; i < CIK_LENGTH; i++)
    {
        if (!(tempCIK[i] >= 'a' && tempCIK[i] <= 'f' || tempCIK[i] >= '0' && tempCIK[i] <= '9'))
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
void Exosite_SetCIK(char * pCIK)
{
    exosite_meta_write((unsigned char *)pCIK, CIK_LENGTH, META_CIK);

    return;
}



/*!
*  \brief  Retrieves a the CIK from NV and places it in to the string pointed
*			at by pCIK
*
* \param[out] pCIK Pointer to CIK
*
*/
void Exosite_GetCIK(char * pCIK)
{
    exosite_meta_read((unsigned char *)pCIK, CIK_LENGTH, META_CIK);

    return;
}


/*!
*  \brief  writes data to Exosite
*
* \param[in] pbuf Pointer to buffer of data to write to Exosite
* \param[in] bufsize length of data in buffer
*
* \return TODO
*
*/
int32_t Exosite_Write(char * pbuf, unsigned char bufsize)
{
    char strBuf[10];
    long sock = -1;

    uint16_t responseLen = 0;

    while (sock < 0)
        sock = connect_to_exosite();



// This is an example write POST...
//  s.send('POST /onep:v1/stack/alias HTTP/1.1\r\n')
//  s.send('Host: m2.exosite.com\r\n')
//  s.send('X-Exosite-CIK: 5046454a9a1666c3acfae63bc854ec1367167815\r\n')
//  s.send('Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n')
//  s.send('Content-Length: 6\r\n\r\n')
//  s.send('temp=2')

    itoa((int)bufsize, strBuf, 10); //make a string for length

    sendLine(sock, POSTDATA_LINE, "/onep:v1/stack/alias");
    sendLine(sock, HOST_LINE, NULL);
    sendLine(sock, CIK_LINE, NULL);
    sendLine(sock, CONTENT_LINE, NULL);
    sendLine(sock, LENGTH_LINE, strBuf);
    exoHAL_SocketSend(sock, pbuf, bufsize);

    char responseStr[255];
    responseLen = getResponse(sock, responseStr, 255);
    exoHAL_SocketClose(sock);

    // 204 "No content"
    if (checkResponse(responseStr, "204"))
    {
        exositeWriteFailures = 0;
    }
    else
        exositeWriteFailures++;


    if (exositeWriteFailures > 5)
    {
        // sometimes transport connect works even if no connection...
        exoHAL_HandleError(EXO_ERROR_WRITE);
    }

    if (!exositeWriteFailures)
    {
        exoHAL_ShowUIMessage(EXO_CLIENT_RW);
        return 1; // success
    }

    return 0;
}



/*!
*  \brief  Reads data from Exosite
*
* \param[in] palias Name of data source alias to read from
* \param[in] pbuf read buffer to put the read response into
* \param[in] buflen Size of buffer
*
* \return length of data in response
*
*/
int32_t Exosite_Read(char * palias, char * pbuf, unsigned char buflen)
{
    unsigned char vlen;
    long sock = -1;

    uint16_t responseLen = 0;

    while (sock < 0)
        sock = connect_to_exosite();


// This is an example read GET
//  s.send('GET /onep:v1/stack/alias?temp HTTP/1.1\r\n')
//  s.send('Host: m2.exosite.com\r\n')
//  s.send('X-Exosite-CIK: 5046454a9a1666c3acfae63bc854ec1367167815\r\n')
//  s.send('Accept: application/x-www-form-urlencoded; charset=utf-8\r\n\r\n')

    sendLine(sock, GETDATA_LINE, palias);
    sendLine(sock, HOST_LINE, NULL);
    sendLine(sock, CIK_LINE, NULL);
    sendLine(sock, ACCEPT_LINE, "\r\n");

    vlen = 0;

    char responseStr[255];
    responseLen = getResponse(sock, responseStr, 255);
    exoHAL_SocketClose(sock);

    // 204 "No content"
    if (checkResponse(responseStr, "200"))
    {
        //find first '\n' char from end of response
        for (int i = responseLen; i > 0; i--)
        {
            // find last \n
            if (responseStr[i] == '\n')
            {
                uint8_t charNotMatch = 0;
                for (int j = 1; (j <= i) && i > 0; j++)
                {
                    // If we're at the end of the inputted string?
                    if (palias[j-1] == '\0')
                    {
                        // if all chars match, we found the key
                        if (!charNotMatch)
                        {
                            // move j passed the '='
                            j++;

                            for (int k = 0;
                                 (k <= buflen) && ((i + j + k) <= responseLen);
                                 k++)
                            {
                                // copy remaining data into buffer
                                pbuf[k] = responseStr[i+j+k];
                                vlen = k;
                            }
                            i = 0;
                        }
                        else
                        {
                            // match not found, exit
                            i = 0;
                            vlen = 0;
                        }
                    }

                    // confirm letter by letter
                    charNotMatch |= !(responseStr[i+j] == palias[j-1]);
                }
            }
        }
    }

    return vlen;
}


/*!
*  \brief  Attempts to activate a device with exosite
*
* \return The results of the activation attempt
*
*/
EXOSITE_DEVICE_ACTIVATION_STATE activate_device(void)
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

    long sock = -1;
    volatile int length;
    char strLen[5];
    uint16_t responseLen = 0;

    EXOSITE_DEVICE_ACTIVATION_STATE retVal = CONNECTION_ERROR;
    

    sock = connect_to_exosite();

    if (sock == 0)
    {
        exoHAL_HandleError(EXO_ERROR_CONNECT);
        while(1);
    }

    char cik[CIK_LENGTH] = {'\0'};

    // get UUID
    exoHAL_ReadMetaItem((unsigned char *)cik, CIK_LENGTH, META_CIK);

    length = strlen((char *)exosite_provision_info) + META_UUID_SIZE;
    itoa(length, strLen, 10); //make a string for length

    sendLine(sock, POSTDATA_LINE, "/provision/activate");
    sendLine(sock, HOST_LINE, NULL);
    sendLine(sock, CONTENT_LINE, NULL);
    sendLine(sock, LENGTH_LINE, strLen);
    sendLine(sock, VENDOR_LINE, NULL);

    char responseStr[255];
    responseLen = getResponse(sock, responseStr, 255);
    exoHAL_SocketClose(sock);

    if (checkResponse(responseStr, "200"))
    {
        // we received a CIK.

        //find first '\n' char from end of response
        for (int i = responseLen; i > 0; i--)
        {
            if (responseStr[i] == '\n')
            {
                // check that we're where we think we should be.
                if ((responseLen-i - 1) != CIK_LENGTH)
                {
                    // the data after the '\n' did not equal the length
                    // of the CIK.  An error of some sorts occured
                    retVal = CONNECTION_ERROR;
                    // end this loop and return our error
                    i = 0;
                }
                else
                {
                    // copy cik into mem.
                    Exosite_SetCIK(&responseStr[i + 1]);
                    retVal = VALID_CIK;
                    i = 0;
                }
            }
        }
    }
    else if (checkResponse(responseStr, "409"))
    {
        // TODO: validate the cik instead of checking the first char for '\0'
        if (cik[0] == '\0')
        {
            // if we don't have a CIK in nvm and we receive a 409
            // The device isn't enabled in the dashboard
            retVal = DEVICE_NOT_ENABLED;
        }
        else
        {
            // If we receive a 409 and we do have a valid CIK, we will
            // assume we are good to go.
            retVal = VALID_CIK;
        }
    }
    else if (checkResponse(responseStr, "401"))
    {
        // RW error
        retVal = R_W_ERROR;
    }

    return retVal;
}



/*!
*  \brief  Checks /ip API to see if a new server IP address should be used
*

* \return none
*
*/
void update_m2ip(void)
{
    //TODO - stubbed out
    return;
}


/*!
*  \brief  Reads UUID from the hardware
*
* \return 0 for failure, uuid_len for success
*
*/
int32_t init_UUID()
{
    unsigned char struuid[25];
    unsigned char uuid_len = 0;

    uuid_len = exoHAL_ReadUUID(struuid);
    exosite_meta_write(struuid, uuid_len, META_UUID);

    return uuid_len;
}



/*!
*  \brief  Establishes a connection with the Exosite API server
*
* \param[in] palias Name of data source alias to read from
* \param[in] pbuf read buffer to put the read response into
* \param[in] buflen Size of buffer
*
* \return success: socket handle; failure: 0;
*
*/
int32_t connect_to_exosite(void)
{
    static unsigned char connectRetries = 0;
    long sock;

    if (connectRetries++ > 5)
    {
        connectRetries = 0;
        exoHAL_HandleError(EXO_ERROR_CONNECT);
    }

    sock = exoHAL_SocketOpenTCP();


    if (sock == -1)
    {
        //wlan_stop();  //TODO - if we stop the wlan, we have to recover somehow...
        exoHAL_MSDelay(100);
        return 0;
    }

    if (exoHAL_ServerConnect(sock) < 0) // Try to connect
    {
        // TODO - the typical reason the connect doesn't work is because
        // something was wrong in the way the comms hardware was initialized (timing, bit
        // error, etc...). There may be a graceful way to kick the hardware
        // back into gear at the right state, but for now, we just
        // return and let the caller retry us if they want
        exoHAL_MSDelay(100);
        return 0;
    }
    else
    {
        connectRetries = 0;
        exoHAL_ShowUIMessage(EXO_SERVER_CONNECTED);
    }

    // Success
    return sock;
}


/*!
*  \brief  Reads first 12 bytes of HTTP response and extracts the 3 byte code
*
* \param[in] socket Pointer to expected HTTP response code
* \param[in] pbuf read buffer to put the read response into
* \param[in] buflen Size of buffer
*
* \return 1 if match, 0 if no match
*
*/
int32_t readResponse(int32_t socket, char * code)
{
    char rxBuf[12];
    int rxLen = 0;

    rxLen = exoHAL_SocketRecv(socket, rxBuf, 12);

    if (12 == rxLen && code[0] == rxBuf[9] && code[1] == rxBuf[10] && code[2] == rxBuf[11])
    {
        return 1;
    }

    return 0;
}

/*!
*  \brief  determines if response matches code
*
* \param[in] code Pointer to expected HTTP response code
* \param[in] response an HTTP response string
*
* \return 1 if match, 0 if no match
*
*/
int32_t checkResponse(char * response, char * code)
{
    if (code[0] == response[9] && code[1] == response[10] && code[2] == response[11])
    {
        return 1;
    }

    return 0;
}



/*!
*  \brief  Retrieves data from a socket
*
* \param[in] socket Socket to retrieve data from
* \param[in] buf Socket to retrieve data from
* \param[in] len Length of buf
*
* \return length of response
*
*/
uint16_t getResponse(int32_t socket, char * buf, uint16_t len)
{
    uint16_t rxLen = 0;

    rxLen = exoHAL_SocketRecv(socket, buf, len);

    return rxLen;
}


/*!
*  \brief  Sends data out the socket
*
* \param[in] socket Socket to send data out ot
* \param[in] LINE type of line going out the socket
* \param[in] payload pointer to data to send out socket
*
* \return 1 if match, 0 if no match
*
*/
void sendLine(int32_t socket, unsigned char LINE, const char * payload)
{
    char strBuf[70];
    unsigned char strLen;

    switch(LINE)
    {
    case CIK_LINE:
        strLen = strlen(STR_CIK_HEADER);
        memcpy(strBuf,STR_CIK_HEADER,strLen);
        exosite_meta_read((unsigned char *)&strBuf[strLen], CIK_LENGTH, META_CIK);
        strLen += CIK_LENGTH;
        memcpy(&strBuf[strLen],STR_CRLF, 2);
        strLen += strlen(STR_CRLF);
        break;
    case HOST_LINE:
        strLen = strlen(STR_HOST);
        memcpy(strBuf,STR_HOST,strLen);
        break;
    case CONTENT_LINE:
        strLen = strlen(STR_CONTENT);
        memcpy(strBuf,STR_CONTENT,strLen);
        break;
    case ACCEPT_LINE:
        strLen = strlen(STR_ACCEPT);
        memcpy(strBuf,STR_ACCEPT,strLen);
        memcpy(&strBuf[strLen],payload, strlen(payload));
        strLen += strlen(payload);
        break;
    case LENGTH_LINE: // Content-Length: NN
        strLen = strlen(STR_CONTENT_LENGTH);
        memcpy(strBuf,STR_CONTENT_LENGTH,strLen);
        memcpy(&strBuf[strLen],payload, strlen(payload));
        strLen += strlen(payload);
        memcpy(&strBuf[strLen],STR_CRLF, 2);
        strLen += 2;
        memcpy(&strBuf[strLen],STR_CRLF, 2);
        strLen += 2;
        break;
    case GETDATA_LINE:
        strLen = strlen(STR_GET_URL);
        memcpy(strBuf,STR_GET_URL,strLen);
        memcpy(&strBuf[strLen],payload, strlen(payload));
        strLen += strlen(payload);
        memcpy(&strBuf[strLen], STR_HTTP, strlen(STR_HTTP));
        strLen += strlen(STR_HTTP);
        break;
    case VENDOR_LINE:
        strLen = strlen((char *)exosite_provision_info);
        memcpy(strBuf, exosite_provision_info, strLen);
        exosite_meta_read((unsigned char *)&strBuf[strLen], META_UUID_SIZE, META_UUID);
        strLen += META_UUID_SIZE;
        break;
    case POSTDATA_LINE:
        strLen = strlen("POST ");
        memcpy(strBuf,"POST ", strLen);
        memcpy(&strBuf[strLen],payload, strlen(payload));
        strLen += strlen(payload);
        memcpy(&strBuf[strLen], STR_HTTP, strlen(STR_HTTP));
        strLen += strlen(STR_HTTP);
        break;
    case EMPTY_LINE:
        strLen = strlen(STR_CRLF);
        memcpy(strBuf,STR_CRLF,strLen);
        break;
    default:
        break;
    }
    exoHAL_SocketSend(socket, strBuf, strLen);

    return;
}



