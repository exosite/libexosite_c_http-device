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
    // do stuff to open socket
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
    // do stuff to open a socket

    return 0;
    
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
    
    // write buffer to socket
    return 0;
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
    // read from socket
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
    // write cik to nvm
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
    // retrieve cik from nvm
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
   
    return 0;
}

/*!
 * \brief memcpy implementation
 */
uint8_t exoPal_memcpy(char * dst, const char * src, uint16_t length)
{

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
    return 0;
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
    return 0;
}

