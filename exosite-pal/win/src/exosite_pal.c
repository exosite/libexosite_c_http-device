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
//#include "exosite.h"
#include "exosite_pal.h"


#include <winsock.h>
#pragma comment(lib, "ws2_32.lib")


// local functions


static SOCKET theSocket;



/*!< Used to reserve memory to emulate NVM during unit testing */
struct UnitTest_meta_storage
{
    char cik[40];
    char uuid[20];
    char vendor[15];
    char model[15];
};


struct UnitTest_meta_storage mem_nvm;

struct dev_nvm
{
    const char * cik;
    const char * uuid;
    const char * vendor;
    const char * model;
    const char * server;
};

struct dev_nvm mem;


uint16_t exoHAL_strlen(const char *s)
{
    return strlen(s);
    
}

/*!
 * \brief Closes a tcp socket
 *
 * 
 *
 * \param[in]
 * \param[out]
 * \return errorcode if failed, else a 0
 * \sa
 * \note
 * \warning
 */
uint8_t exoHAL_tcpSocketClose()
{
    int nRet;
    nRet = closesocket(theSocket);
    if (nRet == SOCKET_ERROR)
    {
        return 1;
    }
    return 0;
}


/*!
 * \brief Opens a tcp socket
 *
 * 
 *
 * \param[in]
 * \param[out]
 * \return errorcode if failed, else a 0
 * \sa
 * \note
 * \warning
 */
uint8_t exoHAL_tcpSocketOpen()
{
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA wsaData;
    int nRet;
    short nPort = 80;


    //
    // Initialize WinSock and check the version
    //
    nRet = WSAStartup(wVersionRequested, &wsaData);

    LPHOSTENT lpHostEntry;

    lpHostEntry = gethostbyname("m2.exosite.com");



    theSocket = socket(AF_INET,// Address family
        SOCK_STREAM,// Socket type
        IPPROTO_TCP);		// Protocol
    if (theSocket == INVALID_SOCKET)
    {
        return 1;
    }

    //
    // Fill in the address structure
    //
    SOCKADDR_IN saServer;

    saServer.sin_family = AF_INET;

    // Server's address
    saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
    saServer.sin_port = htons(nPort);	// Port number from command line

    //
    // connect to the server
    //
   

    nRet = connect(theSocket,	// Socket
        (LPSOCKADDR)&saServer,	// Server address
        sizeof(struct sockaddr));// Length of server address structure
    if (nRet == SOCKET_ERROR)
    {
        return 2;
    }

   

   return 0;
    
}




/*!
 * \brief Sends data to the open tcp socket
 *
 * 
 *
 * \param[in] buffer Data to write to socket
 * \param[in] len Length of data to write to socket
 *
 * \return errorcode if failed, else a 0
 * \sa
 * \note
 * \warning
 */
uint8_t exoHAL_socketWrite( const char * buffer, uint16_t len)
{
    //
    // Send data to the server
    //
    int nRet;

    nRet = send(theSocket,// Connected socket
        buffer,	// Data buffer
        len,	// Length of data
        0);	// Flags
    if (nRet == SOCKET_ERROR)
    {
        return 1;
    }
    return 0;
}


/*!
 * \brief
 *
 * 
 *
 * \param[in] bufferSize Size of buffer
 * \param[out] buffer Buffer received data will be written to
 * \param[out] responseLength amount of data received from modem
 *
 * \return errorcode if failed, else a 0
 * \sa
 * \note len must be greater than sizeof(buffer)
 * \warning
 */
uint8_t exoHAL_socketRead( char * buffer, uint16_t bufSize, uint16_t * responseLength)
{
    //
    // Wait for a reply
    //
    int nRet;
    nRet = recv(theSocket, // Connected socket
        buffer,	// Receive buffer
        bufSize,	// Size of receive buffer
        0);	// Flags
    if (nRet == SOCKET_ERROR)
    {
        return 1;
    }

    *responseLength = nRet;
    return 0;

}




/*!
 * \brief Delay function
 *
 * 
 *
 * \param[in] delay Delay time in ms
 *
 * \return
 * \sa
 * \note
 * \warning
 */
void exoHAL_MSDelay(uint16_t delay)
{
    //MSTimerDelay(delay);

    return;
}





/*!
 * \brief Sets the cik
 *
 * Writes the 40 chars starting at cik* to nvm.
 *
 * \param[in] cik cik to write to nvm
 *
 * \return errorcode if successful else 0
 * \sa
 * \note
 * \warning
 */
uint8_t exoHal_setCik(const char * cik)
{
    memcpy( mem_nvm.cik,cik, sizeof(mem_nvm.cik));
    return 0;
}


/*!
 * \brief
 *
 * 
 *
 * \param[in]
 * \param[out]
 *
 * \return
 * \sa
 * \note
 * \warning
 */
uint8_t exoHal_getCik(char * read_buffer)
{
    memcpy( read_buffer,mem_nvm.cik,sizeof(mem_nvm.cik));
    return 0;
}


/*!
 * \brief
 *
 * 
 *
 * \param[in]
 * \param[out]
 *
 * \return
 * \sa
 * \note
 * \warning
 */
uint8_t exoHal_getModel(char * read_buffer)
{
    memcpy(read_buffer,mem_nvm.model, sizeof(mem_nvm.model));
    return 0;
}


/*!
 * \brief
 *
 * 
 *
 * \param[in]
 * \param[out]
 *
 * \return
 * \sa
 * \note
 * \warning
 */
uint8_t exoHal_getVendor(char * read_buffer)
{
    memcpy(read_buffer,mem_nvm.vendor, sizeof(mem_nvm.vendor));
    return 0;
}


/*!
 * \brief Retrieves UUID from device
 *
 *	This function retrieves a unique ID from your device.  This is typically
 *	the MEID of a cell modem, MAC address of a network card, or serial number
 *	of the device.
 *
 * \param[in] UUID_buf Buffer to put the devices UUID into.
 * \param[out]
 * \return 1 if failed to retrieve UUID, else 0
 * \sa
 * \note
 * \warning
 */
uint8_t exoHal_getUuid(char * read_buffer)
{
    memcpy(read_buffer,mem_nvm.uuid, sizeof(mem_nvm.uuid));
    return 0;
}



/*!
 * \brief
 *
 * 
 *
 * \param[in]
 * \param[out]
 * \return
 * \sa
 * \note
 * \warning
 */
uint8_t exoHAL_itoa(int value, char* str, int radix)
{
    uint8_t strLength = 0;
    static char dig[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";
    int n = 0, neg = 0;
    unsigned int v;
    char* p, *q;
    char c;
    if (radix == 10 && value < 0)
    {
        value = -value;
        neg = 1;
    }
    v = value;
    do
    {
        str[n++] = dig[v%radix];
        v /= radix;
    }
    while (v);
    if (neg)
        str[n++] = '-';
    str[n] = '\0';
    strLength = n-1;
    for (p = str, q = p + n/2; p != q; ++p, --q)
        c = *p, *p = *q, *q = c;
    return strLength;
}

