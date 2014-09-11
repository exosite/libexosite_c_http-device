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
#include "string.h"
#include "stdio.h"
#include "stdlib.h"


/*!
 * Buffer for rx data
 */
char  exoPal_rxBuffer[RX_BUFFER_SIZE];

/*!
 * Used to track how far into the rx buffer we are
 */
static uint16_t bufferCount=0;

/*!
 * Used to track if we currently have a socket open
 */
static uint8_t isSocketOpen = 0;

// local functions

/*!
 * Stores a bunch of stuff to emulate nvm and manipulate return results
 */
struct UnitTest_storage mem_nvm;


/*!
 * \brief Retrieves a pointer to the UnitTest_storage struct
 *
 * This allows external applications speciically unit tests, to manipulate
 * the way the the hal behaves.
 *
 *
 * \return A pointer to the UnitTest_storage
 * \sa
 * \note
 * \warning
 */
void * getUnitTestStorageStruct()
{
    return &mem_nvm;
}


/*!
 * \brief Determines the length of a null terminated string
 *
 * You can write your own, or link to one that is available.
 * This does not include the trailing '\0'. for example, strlen("") == 0 and
 * strlen("A") == 1
 *
 * \param[in] s String you want to determine the length of
 *
 * \return The length of the inputted string.
 * \sa
 * \note
 * \warning  There is no protection for overrun here... Be Careful!
 */
uint16_t exoPal_strlen(const char *s)
{
    return strlen(s);
    
}

/*!
 * \brief Gets the decimal ascii representation of an integer
 *
 * 
 *
 * \param[in] value Number that you want a string representation of
 * \param[in] buf Buffer to place results
 * \param[in] bufSizelength of buffer

 * \return Length of string written to buf
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_itoa(int value, char* buf, uint8_t bufSize)
{
    int32_t chars = sprintf(buf, "%d", value);
    if (chars < 0)
    {
        return 0;
    }
    return chars;
    
}

int32_t exoPal_atoi(char* val)
{
	return atoi(val);
}

/*!
 * \brief A function to copy from one place in memory to another
 *
 * You can write your own or call another.
 *
 * \param[in] dst Memory location of where to write to
 * \param[in] src Memory location of where to copy from
 * \param[in] length Number of bytes to copy
 *
 * \return 0 if failed, else returns dst address.
 * \sa
 * \note
 * \warning
 */
void * exoPal_memcpy(void* dst, const void * src, uint16_t length)
{
    return memcpy(dst,src,length);
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
uint8_t exoPal_tcpSocketClose()
{
    isSocketOpen = 0;
    return mem_nvm.retVal_tcpSocketClose;
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
uint8_t exoPal_tcpSocketOpen()
{
    if (mem_nvm.retVal_tcpSocketOpen == 0)
    {
        isSocketOpen = 1;
        mem_nvm.writeToBufferLen = 0;
    }
    return mem_nvm.retVal_tcpSocketOpen;   
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
uint8_t exoPal_socketWrite( const char * buffer, uint16_t len)
{
    if (isSocketOpen == 1)
    {
        exoPal_memcpy(mem_nvm.writeToBuffer + mem_nvm.writeToBufferLen,buffer,len);
        mem_nvm.writeToBufferLen += len;
    }
    
    return mem_nvm.retVal_socketWrite;   
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
uint8_t exoPal_socketRead( char * buffer, uint16_t bufSize, uint16_t * responseLength)
{
    exoPal_memcpy(buffer,mem_nvm.readFromBuffer,bufSize);
    *responseLength = mem_nvm.readFromBufferLen;
    return mem_nvm.retVal_socketRead;
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
void exoPal_MSDelay(uint16_t delay)
{
    //MSTimerDelay(delay);

    return;
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
uint8_t exoPal_setCik(const char * cik)
{
    memcpy( mem_nvm.cik,cik, sizeof(mem_nvm.cik));
    return mem_nvm.retVal_setCik;
}


/*!
 * \brief Retrieves the stored CIK
 *
 * 
 *
 * \param[out] read_buffer pointer of buffer to place results in
 *
 * \return 0 if successful, else errorCode
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_getCik(char * read_buffer)
{
    memcpy( read_buffer,mem_nvm.cik,sizeof(mem_nvm.cik));
    return mem_nvm.retVal_getCik;
}


/*!
 * \brief Retrieves the stored Model string
 *
 * 
 *
 * \param[in] read_buffer pointer of buffer to place results in
 *
 * \return 0 if successful, else returns error code
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_getModel(char * read_buffer)
{
    memcpy(read_buffer,mem_nvm.model, sizeof(mem_nvm.model));
    return mem_nvm.retVal_getModel;
}


/*!
 * \brief Retrieves the vendor string
 *
 * 
 *
 * \param[in] read_buffer pointer of buffer to place results in
 *
 * \return returns 0 if successful, else returns error code.
 * \sa
 * \note
 * \warning
 */
uint8_t exoPal_getVendor(char * read_buffer)
{
    memcpy(read_buffer,mem_nvm.vendor, sizeof(mem_nvm.vendor));
    return mem_nvm.retVal_getVendor;
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
uint8_t exoPal_getUuid(char * read_buffer)
{
    exoPal_memcpy(read_buffer,mem_nvm.uuid, sizeof(mem_nvm.uuid));
    return mem_nvm.retVal_getUuid;
}

/*!
 * @brief  Used to do any operations before 
 *
 * 
 * @return void
 */void exoPal_sendingComplete()
{
    //printf("[EXOPAL] Sending\r\n");//: %.*s\r\n", exoPal_txBufCounter, exoPal_txBuffer + 45);
    
   //printf("[EXOPAL] Done Sending\r\n");
}

/*!
 * \brief memcpy implementation
 */
uint8_t exoPal_memcpy(char * dst, const char * src, uint16_t length)
{
    memcpy(dst,src,length);
    return 0;
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
