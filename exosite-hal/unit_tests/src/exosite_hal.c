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
#include "exosite_hal.h"
#include "string.h"
#include "stdio.h"

// used to see where we are in the TX buffer
static uint16_t bufferCount=0;

static uint8_t isSocketOpen = 0;

// local functions


struct UnitTest_storage mem_nvm;


void * getUnitTestStorageStruct()
{
    return &mem_nvm;
}



uint16_t exoHal_strlen(const char *s)
{
    return strlen(s);
    
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
uint8_t exoHal_itoa(int value, char* buf, uint8_t bufSize)
{
    int32_t chars = sprintf_s(buf,bufSize, "%d", value);
    if (chars < 0)
    {
        return 0;
    }
    return chars;
    
}



void * exoHal_memcpy(void* dst, const void * src, uint16_t length)
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
uint8_t exoHal_tcpSocketClose()
{
    isSocketOpen = 0;
    bufferCount = 0;
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
uint8_t exoHal_tcpSocketOpen()
{
    if (mem_nvm.retVal_tcpSocketOpen == 0)
    {
        isSocketOpen = 1;
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
uint8_t exoHal_socketWrite( const char * buffer, uint16_t len)
{
    if (isSocketOpen == 1)
    {
        exoHal_memcpy(mem_nvm.writeToBuffer + bufferCount,buffer,len);
        bufferCount += len;
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
uint8_t exoHal_socketRead( char * buffer, uint16_t bufSize, uint16_t * responseLength)
{
    exoHal_memcpy(buffer,mem_nvm.readFromBuffer,bufSize);
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
void exoHal_MSDelay(uint16_t delay)
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
    return mem_nvm.retVal_setCik;
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
    return mem_nvm.retVal_getCik;
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
    return mem_nvm.retVal_getModel;
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
uint8_t exoHal_getUuid(char * read_buffer)
{
    exoHal_memcpy(read_buffer,mem_nvm.uuid, sizeof(mem_nvm.uuid));
    return mem_nvm.retVal_getUuid;
}



