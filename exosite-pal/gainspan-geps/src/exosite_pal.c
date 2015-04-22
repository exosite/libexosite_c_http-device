/*****************************************************************************
*
*  exosite_hal.c - Exosite hardware & environmenat adapation layer.
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

#ifdef GSN_SSL_CLIENT_SUPPORT
#include "gsn_ssl.h"
#endif

#ifndef WIN32
#include "config/app_config_private.h"
//#include "exosite.h"
#include "gsn_otafu.h"

#include "main\app_main_ctx.h"
#else
#include "geps_pal.h"
#endif

// Set to 1 if you want to try and keep a socket open.
#define KEEP_SOCKET_OPEN 1

#define FW_READ_CHUNK_BYTES 512

#define PAL_CIK_LENGTH 40
#define READ_FW_ATTEMPTS 15 // Number of times to attempt getting data from socket
                            // before assuming it's broken

static int32_t SockDes = -1;

// The rxbuffer is also going to be used to buffer tx data before it is sent
char  exoPal_rxBuffer[RX_BUFFER_SIZE];


uint16_t exoPal_txBufCounter = 0;

ttUserIpAddress exoPal_ip = 0;

static char hostname[] = "boss-controls-dev.m2.exosite.com";
static char ContentLengthStr[] = "Content-Length: ";

#ifdef GSN_SSL_CLIENT_SUPPORT
GSN_SSL_CONN_T Ssl;
GSN_SSL_CONN_PARAMS_T params;
#endif


#ifdef WIN32
char cikBuffer[40];
int32_t txBufCounter = 0;
char txBuffer[4096];
#endif

static int32_t openSock()
{
    int32_t sockStatus = 0;
#ifndef WIN32
    struct sockaddr_in tServ_addr;
    // if we try to open when we think we already have one open, try to close it.
    if (SockDes != -1)
    {
        closeSock();
    }

    

    
    exoPal_txBufCounter = 0;

    tServ_addr.sin_family = AF_INET;
#ifdef GSN_SSL_CLIENT_SUPPORT
    tServ_addr.sin_port = htons(443);
#else
    tServ_addr.sin_port = htons(80);
#endif
    //C0A80A8A debian vm
    tServ_addr.sin_addr.s_addr = exoPal_ip;//htonl(0xC0A80A88);//Desktop htonl(0xC0A80A8A);// htonl(0xC0A80339);//htonl(0xC0A8037B)/* vm */;//


    // do stuff to open a socket
    SockDes = socket(AF_INET, SOCK_STREAM, 0);
    printf("[EXOPAL] Creating socket\r\n");
    /* set the tServ_addr socket structure and connect*/
    sockStatus = connect(SockDes, (struct sockaddr*)&tServ_addr, sizeof(tServ_addr));

    printf("[EXOPAL] Socket create status: %d\r\n", sockStatus);
    if (sockStatus != 0)
    {
        
        return 1;
    }

#ifdef GSN_SSL_CLIENT_SUPPORT
    params.caCertLen = 0;
    printf("[EXOPAL] starting ssl\r\n");
#warning docs says this should be 1 if valid, but appears to return zero if valid...
    sockStatus = GsnSsl_Open(&Ssl, SockDes, &params);
    printf("[EXOPAL] SSL create status: %d\r\n", sockStatus);
    if (sockStatus != 0)
    {
        // Something happened in the ssl open, close it and try again next time.
        printf("[EXOPAL] Returning error: %d\r\n", sockStatus);
        closeSock();
        return 2;
    }
#endif
#endif   
    return 0;
}

/*!
 * \brief Closes a tcp socket
 *
 * Closes the currently open socket.  This can be done either through
 * an OS, or direct calls to the modem.
 *
 * \return 0 if successful, else error code
 * \sa exoPal_tcpSocketOpen
 */
uint8_t exoPal_tcpSocketClose()
{
    exoPal_txBufCounter = 0;
    if (KEEP_SOCKET_OPEN == 0)
    {
        // do stuff to close socket
        closeSock();
    }
    return 0;
}

static int32_t closeSock()
{
#ifndef WIN32
#ifdef GSN_SSL_CLIENT_SUPPORT
    GsnSsl_Close(&Ssl);
#endif
    tfClose(SockDes);
    printf("[EXOPAL] socket closed\r\n");
    SockDes = -1;

#endif
    return 0;
}

int32_t exoPal_recv(int32_t socketDescriptor, char * bufferPtr, int bufferLength, int flags)
{
    int32_t response;
    uint32_t dataLen= 0;
    uint8_t *rxbuf;

    if (SockDes < 0)
    {
        return -100;
    }

#ifdef GSN_SSL_CLIENT_SUPPORT
#warning Why the hell are you casting a uint16 to a uint32 ? !
    

    response = GsnSsl_DataReceive(&Ssl, SockDes, &rxbuf, &dataLen, 10);

    if (response == GSN_FAILURE)
    {
        printf("[EXOPAL] Error receiving: %d\r\n", response);
        return -101;
    }
    else
    {
        response = dataLen;
    }
    memmove(bufferPtr, rxbuf, dataLen);
    GsnSsl_Free(rxbuf);

#else
    response = recv(socketDescriptor, bufferPtr, bufferLength, flags);
#endif
    return response;
}

/*!
 * \brief Opens a tcp socket
 *
 * Opens the currently open socket.  This can be done either through
 * an OS, or direct calls to the modem.
 *
 * \return 0 if successful, else error code
 *
 * \sa exoPal_tcpSocketClose
 */
uint8_t exoPal_tcpSocketOpen()
{
    if (SockDes == -1)
    {
        printf("Opening socket\r\n");
        return openSock();
    }
    else
    {
        printf("socket already open\r\n");
        // assume socket is already open.
        return 0;
    }
    
    
}




uint32_t exoPal_bossBroadcast()
{
#ifndef WIN32
static int32_t heartBeatSocket = -1;
    const char * res;
    struct sockaddr_in tServ_addr;
    char ipAddrStr[16];
    GSN_NWIF_IP_CONFIG_T nwParams;
    uint8_t *ptMAC;
    
    char heartbeatPacket[] = "HB:XXXXXXXXXXXX\r\n{\"ip\":\"xxx.xxx.xxx.xxx\"}"; // max size of string
                                            // we'll truncate off if ip
                                            // isn't full size.
    int8_t startofIp = 24;  // chars in heartbeatPacket where IP starts
    int8_t startofMac = 3; // chars in heartbeatPacket where MAC starts
    int8_t endofMac = 15;
    int32_t packetLength = 0;
    int32_t sockStatus = 0;
    // get mac
    ptMAC = GsnFactDflt_MacGet();
    
    // copy mac into string
    sprintf(heartbeatPacket + startofMac, "%02X%02X%02X%02X%02X%02X", ptMAC[0],ptMAC[1],ptMAC[2],ptMAC[3],ptMAC[4],ptMAC[5]);
    
    //  remove null terminate and put in the \r 
    heartbeatPacket[endofMac] = '\r';
    
    tServ_addr.sin_family = AF_INET;
    tServ_addr.sin_port = htons(8257);
    
     // Get IP config info
    GsnNwIf_IpConfigGet(&appCtx.appSmCtx.ncm.appNwIf.nwIf, &nwParams);

    // copy ip string into heartbeatPacket
    //res = inet_ntop(AF_INET, &nwParams.ipAddr, heartbeatPacket + startofIp, 16);
    
    sprintf(heartbeatPacket + startofIp, "%08x", htonl(nwParams.ipAddr));

    //sprintf()

    //if (res == NULL)
    //{
    //    // Error in getting IP string
    //    printf("[EXOPAL] Error in getting IP string\r\n");
    //    return -1;
    //}
    
    // get end of ipAddr
    packetLength = strlen(heartbeatPacket);

    // put port on packet, hardcoded to 80
    strcpy(heartbeatPacket + packetLength, "\",\"port\":80}");

   
    
    // set broadcast address for our current subnet
    tServ_addr.sin_addr.s_addr = nwParams.ipAddr | ~nwParams.subnetMask;
    //printf("[boss] broadcast IP: %u\r\n", nwParams.ipAddr | ~nwParams.subnetMask);
    //printf("[boss] broadcast packet: %s\r\n", heartbeatPacket);
    
    if (heartBeatSocket == -1)
    {
        // Open socket if it wasn't already
        heartBeatSocket = socket(AF_INET, SOCK_DGRAM, 0 );
        printf("[EXOPAL] Creating socket\r\n");
       
    }
    printf("[EXOPAL] Socket create status: %d\r\n", sockStatus);
    if (heartBeatSocket == -1)
    {
        printf("[EXOPAL] Error creating socket\r\n");
        return -2;
    }
    
    // send heartbeat
    sendto(heartBeatSocket, heartbeatPacket,strlen(heartbeatPacket),0,(struct sockaddr *)&tServ_addr, sizeof(struct sockaddr_in));
#endif    
    return 0;
}


/*!
 * \brief
 *
 * Any HW or SW initialization should be performed in here
 *
 * This function is meant to perform any one time initialization and/or setup.
 * This will be called everytime Exosite_init is called.  An example use for
 * this function would be to perform a DNS lookup of m2.exosite.com
 *
 */
void exoPal_init()
{
    return;
}

/*!
 * \brief Sends data to the open tcp socket
 *
 * Write data out to the currently open socket
 *
 * \param[in] buffer Data to write to socket
 * \param[in] len Length of data to write to socket
 *
 * \sa exoPal_socketRead
 *
 * \return 0 if successful, else error code
 */
uint8_t exoPal_socketWrite( const char * buffer, uint16_t len)
{
#ifndef WIN32
    // check if socket is open
    if (SockDes == -1)
    {
        printf("Tried to write to closed socket");
        // triedt to write with socket closed
        return -1;
    }
    
    
    if((exoPal_txBufCounter + len) > TX_BUFFER_SIZE)
    {
        // tried to write more data than the buffer could hold, send out
        // existing buffer and start over.
        printf("[EXO PAL] tx buffer full, sending and flushing");
        exoPal_sendingComplete();
        exoPal_txBufCounter = 0;
    }
    
    memcpy(exoPal_rxBuffer + exoPal_txBufCounter, buffer, len);
    exoPal_txBufCounter += len;
#else
    memcpy(txBuffer + txBufCounter, buffer, len);
    txBufCounter += len;
#endif
    
    
    return 0;
}

char * exoPal_getHostName()
{
    return hostname;
}

/*!
 * \brief
 *
 * Reads data from a socket, into \a buffer
 *
 * \param[in] bufferSize Size of buffer
 * \param[out] buffer Buffer received data will be written to
 * \param[out] responseLength amount of data received from modem
 *
 *
 * \sa exoPal_socketWrite
 *
 * \note len must be greater than sizeof(buffer)
 *
 * \return 0 if successful, else error code
 */
uint8_t exoPal_socketRead(char * buffer, uint16_t bufferSize, uint16_t * responseLength)
{
    int32_t response;
    int i = 0;
    int32_t dataLen= 0;
    


    response = exoPal_recv(SockDes, buffer, bufferSize, 0);

    if (response < 0)
    {
        printf("[EXOPAL] Closing socket, error receiving %d\r\n", response);
        closeSock();
        return response;
    }

    printf("[EXOPAL] Received %d Bytes\r\n", response);
    printf("[EXOPAL] Contents:\r\n%.*s", 100,buffer);
    for(i = 100; (i < response); i += 100)
    {
        if ((response - i) < 100)
        {
            printf("%.*s", response - i,buffer + i);
        }
        else
        {
            printf("%.*s", 100,buffer + i);
        }
    }
    printf("\r\n");
    if (response >= 0)
    {
        *responseLength = response;
    }
    return 0;

}

/*!
 * @brief  Gets the content length of an http request.
 *
 * @param response [in] HTTP headers string
 * @param bodyLength [out] Length of the body
 * 
 * @return int32_t 0 if successful, else negative
 */
int32_t exopal_getContentLength(char *response, int32_t *bodyLength)
{
    // find content length
    char* strStart;
    char* charAfterContentLengthValue = 0;
    char cr[] = "\r";
    // find start of content length header
    strStart = exoPal_strstr(response, ContentLengthStr);
    if (strStart <= 0)
    {
        return -1;
    }
    strStart = strStart + sizeof(ContentLengthStr) - 1;
    // get \r and set to '\0' for atoi
    charAfterContentLengthValue = exoPal_strstr(strStart, cr);
    if (charAfterContentLengthValue <= 0)
    {
        return -2;
    }
    if (charAfterContentLengthValue != 0)
    {
        
        // temporarily null terminate the content length
        *charAfterContentLengthValue = '\0';
        *bodyLength = exoPal_atoi(strStart);
        *charAfterContentLengthValue = '\r';
       
    }
    return 0;
}

extern const char content_request_str[];

int32_t makeRequestForNextRange(int32_t startingpoint, int32_t length, char * baseName, int32_t baseNameLength, GSN_FWUP_ID_T fwApp)
{
    char modelStr[MAX_MODEL_LENGTH + 1];
    char vendorStr[MAX_VENDOR_LENGTH + 1];
    char * hostName;
 
    char cik[PAL_CIK_LENGTH];


    // assumes max size of range header would be "bytes=aaaaa-bbbbb"
    char rangeHeader[20];

    exoPal_getCik(cik);

    hostName = exoPal_getHostName();
    exoPal_getVendor(vendorStr);

    exoPal_getModel(modelStr);

    exoPal_socketWrite(content_request_str, strlen(content_request_str));
    exoPal_socketWrite(vendorStr, strlen(vendorStr));
    exoPal_socketWrite("&model=", sizeof("&model=") - 1);
    exoPal_socketWrite(modelStr, strlen(modelStr));
    exoPal_socketWrite("&id=", sizeof("&id=") - 1);

    exoPal_socketWrite(baseName, baseNameLength);

    if (fwApp == GSN_FW_APP_0)
    {
        exoPal_socketWrite(APP1_EXTENSION, sizeof(APP1_EXTENSION)-1);
    }
    else if (fwApp == GSN_FW_APP_1)
    {
        exoPal_socketWrite(APP2_EXTENSION, sizeof(APP2_EXTENSION)-1);
    }
    else
    {
        // passed invalid app...
    }

    exoPal_socketWrite(" HTTP/1.1", sizeof(" HTTP/1.1") - 1);
    exoPal_socketWrite("\r\nHost: ", sizeof("\r\nHost: ") - 1);
    exoPal_socketWrite(hostName, strlen(hostName));
    exoPal_socketWrite("\r\nX-Exosite-CIK: ", sizeof("\r\nX-Exosite-CIK: ") - 1);
    exoPal_socketWrite(cik, CIK_LENGTH);
    exoPal_socketWrite("\r\nRange: ", sizeof("\r\nRange: ") - 1);
    // - 1 since range header is inclusive.
    sprintf((char*)rangeHeader, "bytes=%d-%d", startingpoint, startingpoint + length - 1);
    exoPal_socketWrite(rangeHeader, strlen(rangeHeader));
    exoPal_socketWrite("\r\n\r\n", sizeof("\r\n\r\n") - 1);


    exoPal_sendDone(0);

    return 0;
}


/*!
 * \brief
 *
 * Reads data from a socket and loads into into External Flash for an OTAFU
 *
 * \param[in] bufferSize Size of buffer
 * \param[out] buffer Buffer received data will be written to
 * \param[out] responseLength amount of data received from modem
 *
 *
 * \sa exoPal_socketWrite
 *
 * \note len must be greater than sizeof(buffer)
 *
 * \return 0 if successful, else error code
 */
uint8_t exoPal_socketReadFw( char * buffer, 
                                uint16_t bufferSize, 
                                GSN_EXTFLASH_FWUP_CTX *pCtx,
                                GSN_FWUP_ID_T app,
                                char * baseName,
                                int32_t baseNameLength)
{


    int32_t response;
    int i = 0;
    char * bodyStart;
    int32_t contentLength = 0;
    int32_t contentLengthRes = 0;
    int32_t bodyOffset;
    uint8_t spaceFound = 0;
    int16_t j = 0;
    int32_t k = 0;
    char validResponseCode[] = "206";
    char validResponseCode416[] = "416";
    //int32_t validResponse = 0;
    int32_t recvSize = bufferSize;
    int32_t received416 = 0;
    //int32_t receivedError = 0;


    makeRequestForNextRange(0, FW_READ_CHUNK_BYTES, baseName, baseNameLength, app);

    // read 512 bytes at a time until we receive a 416 response
    for (i = 0; received416 == 0; )
    {
        recvSize = exoPal_recv(SockDes, buffer, bufferSize,0);
        if (recvSize < 0)
        {
            return 5;
        }

        //i += recvSize;


        bodyStart = strstr(buffer, "\r\n\r\n") + sizeof("\r\n\r\n") - 1;

        // bodyStart was outside the range of buffer.
        if ((bodyStart < buffer) || (bodyStart > (buffer + bufferSize)))
        {
            return 1;
        }


        ////////////// Check HTTP Response code ////////////////////
        // get start second ' ' separate column
        // assumes will always be before the 15th char
        // assumes first char isn't a ' '
        for (j = 1; (j < 15) && (spaceFound == 0); j++)
        {
            if (buffer[j] == ' ')
            {
                spaceFound = j + 1;
            }
        }
        if (spaceFound > 0)
        {
            bodyOffset = bodyStart - buffer;

            // If we found a ' ', try to match the code
            if (validResponseCode[0] == buffer[spaceFound] && 
                validResponseCode[1] == buffer[spaceFound + 1] && 
                validResponseCode[2] == buffer[spaceFound + 2])
            {
                //validResponse = 1;
                printf("*");
                i = i + FW_READ_CHUNK_BYTES;
                GsnOtafu_FwupContinue(pCtx, (UINT8 *)(buffer + bodyOffset), recvSize - bodyOffset, app);
                // send next request 
                makeRequestForNextRange(i, FW_READ_CHUNK_BYTES, baseName, baseNameLength, app);
            }
            else if (   validResponseCode416[0] == buffer[spaceFound] && 
                        validResponseCode416[1] == buffer[spaceFound + 1] && 
                        validResponseCode416[2] == buffer[spaceFound + 2])
            {
                received416 = 1;
                printf("[EXOPAL] Received 416\r\n");
                GsnOtafu_FwupContinue(pCtx, (UINT8 *)(buffer + bodyOffset), recvSize - bodyOffset, app);
            }
            else
            {
                //receivedError = 1;
                printf("[EXOPAL] Received error\r\n");
                return 7;
            }
        }
      
    }



      
    if (received416 == 0)
    {
        printf("[EXOPAL] Didn't download full package.\r\n", i);
        return 4;
    }
    
    printf("[EXOPAL] Received %d Bytes\r\n", i);
    //GsnFwupExtFlash_DwndEnd

    return 0;

}



/*!
 * \brief Sets the cik
 *
 * Writes the 40 chars starting at cik* to nvm.
 *
 * \param[in] cik cik to write to nvm
 *
 * \return 0 if successful, else error code
 */
uint8_t exoPal_setCik(const char * cik)
{
 
    int32_t rtn;
#ifndef WIN32

    printf("[EXOPAL] Setting cik: %.*s\r\n", CIK_LENGTH, cik);
    boss_exosite_cloud_setCik(cik);
    
    // write to nvm
    rtn = GsnNvds_Write(APP_CFG_NVDS_NCM_BOSS_CIK_ID, 0, PAL_CIK_LENGTH, (void *)cik);
    
    if (rtn)
    {
        printf("[EXOPAL] *** CIK write to NVM failed: %d\r\n", rtn);
    }
#else
    memcpy(cikBuffer, cik, CIK_LENGTH);
    rtn = 0;
#endif
    
    return rtn;
}


/*!
 * \brief Retrieves the cik from nvm
 *
 * The CIK must persist through power cycles and therefore must be in some
 * sort of NVM
 *
 * \param[out] read_buffer Buffer to write data to, must be at least 40 chars wide
 *
 * \return 0 if successful, else error code
 */
uint8_t exoPal_getCik(char * read_buffer)
{
    int32_t rtn;
    static int32_t displayOnce = 0;
    
#ifndef WIN32
    rtn = GsnNvds_Read(APP_CFG_NVDS_NCM_BOSS_CIK_ID, 0, PAL_CIK_LENGTH, read_buffer);
#else
    memcpy(read_buffer, cikBuffer, CIK_LENGTH);
    rtn = 0;
#endif // !WIN32

    boss_exosite_cloud_setCik(read_buffer);

    if (rtn)
    {
        printf("[EXOPAL] *** CIK read from NVM failed: %d\r\n", rtn);
    }
    else if (displayOnce == 0)
    {
        // we're only going to display this the first time we read
        displayOnce = 1;
        printf("[EXOPAL] Retrieved cik: %.*s\r\n", PAL_CIK_LENGTH, read_buffer);
    }

    return rtn;
}


/*!
 * \brief Retrieves the device model
 *
 * This can either be stored in nvm, or hardcoded into this function.  In most 
 * cases, you will want to hardcode this value
 *
 *
 * \param[out] read_buffer Buffer to place contents of model string in.
 *
 * \warning It is up to the user to make sure that \a read_buffer is large
 *          enough to fit the model in.  The size of this buffer can be
 *          adjusted inside exosite.h (MAX_MODEL_LENGTH)
 * \return 0 if successful, else error code
 */
uint8_t exoPal_getModel(char * read_buffer)
{
    
    char uuidBuffer[sizeof("xx-xxxxxx-xxxxxx-xxxxx")];
    exoPal_getUuid(uuidBuffer);
    
    if (uuidBuffer[0] == 'A')
    {
        strcpy(read_buffer, "smart_plug");
    }
    else
    {
        strcpy(read_buffer, "smart_plug_220");
    }
    return 0;
}


/*!
 * \brief Retrieves the device vendor
 *
 * This can either be stored in nvm, or hardcoded into this function.  In most 
 * cases, you will want to hardcode this value
 *
 *
 * \param[out] read_buffer Buffer to place contents of vendor string in.
 *
 * \warning It is up to the user to make sure that \a read_buffer is large
 *          enough to fit the model in.  The size of this buffer can be
 *          adjusted inside exosite.h (MAX_VENDOR_LENGTH)
 * \return 0 if successful, else error code
 */
uint8_t exoPal_getVendor(char * read_buffer)
{
    strcpy(read_buffer, "boss-controls-dev");
    return 0;
}


/*!
 * \brief Retrieves UUID from device
 *
 *	This function retrieves a unique ID from your device.  This is typically
 *	the MEID of a cell modem, MAC address of a network card, or serial number
 *	of the device.
 *
 * \param[in] read_buffer Buffer to put the devices UUID into.
 *
 * \return 0 if successful, else error code
 */
uint8_t exoPal_getUuid(char * read_buffer)
{
#ifndef WIN32
    uint8_t *ptMAC;
    GSN_FACT_DFLT_ELEMENT_T *pFactDfltElmnt;
    char randArr[6];
    static int32_t displayOnce = 0;
    ptMAC = GsnFactDflt_MacGet();
    
    sprintf(read_buffer, "XX-%02X%02X%02X-%02X%02X%02X-xxxxx", ptMAC[0],ptMAC[1],ptMAC[2],ptMAC[3],ptMAC[4],ptMAC[5]);
    
    //Get "random" portion of serial number
    pFactDfltElmnt = GsnFactDflt_ElementGet(GSN_FACT_DFLT_PROV_PASSPHRASE_ID);
    if(pFactDfltElmnt != NULL)
    {
        // Copy model type
        memcpy( read_buffer, pFactDfltElmnt->pVal, 2);
        
        // Copy 5 bytes (to salt the serial number)
        memcpy( read_buffer + 17, pFactDfltElmnt->pVal + 3, 5);
    }
    
    if (displayOnce == 0)
    {
        displayOnce = 1;
        printf("[EXOPAL] Retrieved SN: %s\r\n", read_buffer);
    }
#endif
    return 0;
}


exoPal_sendDone(int32_t debugOutput)
{
#ifndef WIN32
    int32_t i = 0;
    char *outBuf;
    uint32_t len = 0;
    int32_t retVal;
    size_t retSize = 0;
    if (debugOutput)
    {
        printf("[EXOPAL] Sending: ");

        for (i = 0; (i < exoPal_txBufCounter); i += 100)
        {
            if ((exoPal_txBufCounter - i) < 100)
            {
                printf("%.*s", exoPal_txBufCounter - i, exoPal_rxBuffer + i);
            }
            else
            {
                printf("%.*s", 100, exoPal_rxBuffer + i);
            }
        }
        printf("\r\n");
    }
#ifdef GSN_SSL_CLIENT_SUPPORT
    retVal = GsnSsl_Encode(&Ssl, (uint8_t *)exoPal_rxBuffer, exoPal_txBufCounter, (uint8_t **)&outBuf, &len);
    
    if(debugOutput)
    {
        printf("Encoding response: %d\r\n", retVal);
    }
    //send(SockDes, outBuf, len, 0);
    //GsnSsl_Encode(&Ssl, (uint8_t *)exoPal_rxBuffer, exoPal_txBufCounter, (uint8_t **)&outBuf, &len);

    retSize = send(SockDes, outBuf, len, 0);

    if (retSize == -1)
    {
        // try opening the socket and make another attempt
        openSock();
        send(SockDes, outBuf, len, 0);
    }

    GsnSsl_Free((uint8_t *)outBuf);

#else
    send(SockDes, exoPal_rxBuffer, exoPal_txBufCounter, 0);
#endif
#else
    txBufCounter = 0;
#endif
    exoPal_txBufCounter = 0;

    if (debugOutput)
    {
        printf("\r\n[EXOPAL] Done Sending\r\n");
    }
    return 0;
}

/*!
 * @brief  Used to do any operations before 
 *
 * 
 * @return 0 if successful
 */
int32_t exoPal_sendingComplete()
{
    return exoPal_sendDone(1);
}

/*!
 * \brief memcpy implementation
 */
uint8_t exoPal_memcpy(char * dst, const char * src, uint16_t length)
{
    memcpy(dst,src,length);
    return 0;
}

/*!
 * \brief returns the length of the null terminated string
 *
 * This can link to a strlen elsewhere in your code, or you can write 
 * your own.
 *
 * \param[in] s String whose length you want to check
 *
 * \return Length of the string \a s, not including the null terminator.
 * 
 * \warning It is up to the user to make sure that strlen doesn't overrun.
 *          (e.g. pass a non null terminated string)
 */
uint16_t exoPal_strlen(const char *s)
{
    return strlen(s);
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int32_t exoPal_atoi(char* val)
{
    int32_t temp;
    temp = atoi(val);
    return temp;
}

char* exoPal_strstr(const char *in, const char *str)
{
	char c;
	size_t len;

	c = *str++;
	if (!c)
		return (char *) in;	// Trivial empty string case

	len = strlen(str);
	do {
		char sc;

		do {
			sc = *in++;
			if (!sc)
				return (char *) 0;
		} while (sc != c);
	} while (strncmp(in, str, len) != 0);

	return (char *) (in - 1);
}


/*!
 * \brief Gets the decimal ascii representation of an integer
 *
 * 
 *
 * \param[in] value Number that you want a string representation of
 * \param[in] buf Buffer to place results
 * \param[in] bufSize Length of buffer

 * \return Length of string written to buf
 */
uint8_t exoPal_itoa(int value, char* buf, uint8_t bufSize)
{
    
    int i, sign;

    if ((sign = value) < 0)   /* record sign */
        value = -value;           /* make n positive */
    i = 0;
    do          /* generate digits in reverse order */
    {
        buf[i++] = value % 10 + '0';    /* get next digit */
    }
    while ((value /= 10) > 0);        /* delete it */
    if (sign < 0)
        buf[i++] = '-';
    buf[i] = '\0';
    reverse(buf);
    return i;
}

