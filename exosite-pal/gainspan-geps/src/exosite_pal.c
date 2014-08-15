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
#include "config/app_config_private.h"
#include "exosite.h"

static int32_t SockDes;

char  exoPal_rxBuffer[RX_BUFFER_SIZE];
char exoPal_txBuffer[1024];
uint16_t exoPal_txBufCounter = 0;

ttUserIpAddress exoPal_ip = 0;

static char hostname[] = "boss-controls-dev.m2.exosite.com";


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
    // do stuff to close socket
    tfClose(SockDes);
    printf("[EXOPAL] socket closed\r\n");
    SockDes = -1;
    return 0;
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
    struct sockaddr_in tServ_addr;
    
    int32_t sockStatus = 0;
    exoPal_txBufCounter = 0;
    
    tServ_addr.sin_family = AF_INET;
    tServ_addr.sin_port = htons(80);
    tServ_addr.sin_addr.s_addr = exoPal_ip;// htonl(0xC0A80339);//htonl(0xC0A8037B)/* vm */;//
    
    
    // do stuff to open a socket
    SockDes = socket(AF_INET, SOCK_STREAM, 0 );
    printf("[EXOPAL] Socket Create\r\n");
    /* set the tServ_addr socket structure and connect*/
    sockStatus = connect(SockDes, ( struct sockaddr* )&tServ_addr, sizeof( tServ_addr ));
    
    printf("[EXOPAL] Socket Connected?: %d\r\n", sockStatus);
    
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
    // check if socket is open
    if (SockDes == -1)
    {
        // triedt to write with socket closed
        return -1;
    }
    memcpy(exoPal_txBuffer + exoPal_txBufCounter, buffer, len);
    exoPal_txBufCounter += len;

    
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
uint8_t exoPal_socketRead( char * buffer, uint16_t bufferSize, uint16_t * responseLength)
{
    int32_t response;
    int i = 0;
    
    response = recv(SockDes, buffer, bufferSize,0);
    printf("[EXOPAL] Received %d Bytes\r\n", response);
    printf("[EXOPAL] Contents:\r\n%.*s", 100,buffer);
    //for(i = 100; ((i - 100) < response); i += 100)
    //{
    //    printf("%.*s", 100,buffer + i);
    //}
    printf("\r\n");
    if (response >= 0)
    {
        *responseLength = response;
    }
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
    printf("[EXOPAL] Setting cik: %.*s\r\n", 40, cik);
    boss_app_setCik(cik);
    
    // write to nvm
    GsnNvds_Write(APP_CFG_NVDS_NCM_BOSS_CIK_ID, 0, CIK_LENGTH, (void *)cik);
    return 0;
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
    GsnNvds_Read(APP_CFG_NVDS_NCM_BOSS_CIK_ID, 0, CIK_LENGTH, read_buffer);
    boss_app_setCik(read_buffer);
    
    printf("[EXOPAL] Retrieved cik: %.*s\r\n", CIK_LENGTH, read_buffer);
    return 0;
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
    strcpy(read_buffer, "smart_plug");
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
    GSN_FACT_DFLT_ELEMENT_T *ptSN;
    ptSN = GsnFactDflt_ElementGet(GSN_FACT_DFLT_BOSS_SN);
    strcpy(read_buffer, (const char *)ptSN->pVal);
    return 0;
}

/*!
 * @brief  Used to do any operations before 
 *
 * 
 * @return void
 */void exoPal_sendingComplete()
{
    printf("[EXOPAL] Sending");//: %.*s\r\n", exoPal_txBufCounter, exoPal_txBuffer);
    send(SockDes, exoPal_txBuffer, exoPal_txBufCounter, 0);
    printf("[EXOPAL] Done Sending\r\n");
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

