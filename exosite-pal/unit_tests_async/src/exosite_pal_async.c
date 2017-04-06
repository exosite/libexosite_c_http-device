/*****************************************************************************
*
*  exosite_pal_async.c - Common header for Exosite hardware adapation layer
*  Copyright (C) 2017 Exosite LLC
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
#include "exosite_pal_async.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*****************************************************************************
 * \defgroup Unit Test Specific details
 * @{
 */

#define RX_TX_BUFFER_SIZE 512

// unit test specific stuffs
struct UnitTest_storage
{
    char cik[40];
    char uuid[41];
    char vendor[21];
    char model[21];
    char writeToBuffer[RX_TX_BUFFER_SIZE];
    uint16_t writeToBufferLen;
    char readFromBuffer[RX_TX_BUFFER_SIZE];
    uint16_t readFromBufferLen;
    uint8_t retVal_setCik;
    uint8_t retVal_getCik;
    uint8_t retVal_getModel;
    uint8_t retVal_getVendor;
    uint8_t retVal_getUuid;

    uint8_t retVal_start;
    uint8_t retVal_stop;

    uint8_t retVal_tcpSocketClose;
    uint8_t retVal_tcpSocketOpen;
    uint8_t retVal_socketRead;
    uint8_t retVal_socketWrite;
    uint8_t retVal_sendingComplete;
};

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
 * \return A pointer to the UnitTest_storage
 */
void * getUnitTestStorageStruct()
{
    return &mem_nvm;
}

/**@}*/

/*****************************************************************************
 * \defgroup Utility PAL functions
 * @{
 */

/** \brief Get string form of an integer
 * \param[in] value Number that you want a string representation of
 * \param[out] buf Buffer to place results
 * \param[in] bufSizelength of buffer
 * \return Length of string written to buf
 */
uint8_t exoPal_itoa(int value, char* buf, uint8_t bufSize)
{
    int32_t chars = snprintf(buf, bufSize, "%d", value);
    if (chars < 0)
    {
        return 0;
    }
    return chars;
}

/** \brief Get interger from a string.
 * \param[in] val String to get integer from.
 * \return integer or 0
 */
int32_t exoPal_atoi(char* val)
{
    return atoi(val);
}

/** \brief Compute length of string
 * \param[in] s String to compute length of
 * \return Length of string not including NUL
 */
size_t exoPal_strlen(const char *s)
{
    return strlen(s);
}

/** \brief Find string within string
 * \param[in] str String to search within
 * \param[in] target The string to find
 * \return NULL if not found, otherwise pointer to where found
 */
char* exoPal_strstr(const char *str, const char *target)
{
    return strstr(str, target);
}

/** \brief Copy memory from possibly overlapping regions
 * \param[out] dst Where to copy to
 * \param[in] src Where to copy from
 * \param[in] length How much to copy
 * \return dst
 */
void * exoPal_memmove(void* dst, const void * src, size_t length)
{
    return memmove(dst, src, length);
}

/** \brief Write bytes of value
 * \param[out] dst Where to write
 * \param[in] c What byte value to write
 * \param[in] length How many bytes to write with #c
 * \return dst
 */
void * exoPal_memset(void* dst, int c, uint16_t length)
{
    return memset(dst, c, length);
}

/** \brief Copy string
 * \param[out] dst Where to copy to
 * \param[in] src What to copy
 * \param[in] len Maximum size of the buffer #dst
 *
 * This copies at most #len - 1 bytes from #src.
 *
 * \return length of copied string.
 */
size_t exoPal_strlcpy(char* dst, const char* src, size_t len)
{
    return strlcpy(dst, src, len);
}

/** \brief Concatenate strings
 * \param[in,out] dst Where to concatenate onto
 * \param[in] src What to copy
 * \param[in] len Maximum size of the buffer #dst
 *
 * This copies at most #len - strlen(#dst) - 1 bytes from #src.
 *
 * \return length of concatenated string.
 */
size_t exoPal_strlcat(char* dst, const char* src, size_t len)
{
    return strlcat(dst, src, len);
}

/**@}*/

/*****************************************************************************
 * \defgroup Memory/NVRAM/Flash PAL
 * @{
 */

/*!
 * \brief Sets the cik
 *
 * Writes the 40 chars starting at cik* to nvm.
 *
 * \param[in] cik cik to write to nvm
 * \return errorcode if successful else 0
 */
uint8_t exoPal_setCik(const char * cik)
{
    memcpy( mem_nvm.cik,cik, sizeof(mem_nvm.cik));
    return mem_nvm.retVal_setCik;
}

/*!
 * \brief Retrieves the stored CIK
 * \param[out] read_buffer pointer of buffer to place results in
 * \return 0 if successful, else errorCode
 */
uint8_t exoPal_getCik(char * read_buffer)
{
    memcpy( read_buffer,mem_nvm.cik,sizeof(mem_nvm.cik));
    return mem_nvm.retVal_getCik;
}

/*!
 * \brief Retrieves the stored Model string
 * \param[in] read_buffer pointer of buffer to place results in
 * \return 0 if successful, else returns error code
 */
uint8_t exoPal_getModel(char * read_buffer)
{
    memcpy(read_buffer,mem_nvm.model, sizeof(mem_nvm.model));
    return mem_nvm.retVal_getModel;
}

/*!
 * \brief Retrieves the vendor string
 * \param[in] read_buffer pointer of buffer to place results in
 * \return returns 0 if successful, else returns error code.
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
 */
uint8_t exoPal_getUuid(char * read_buffer)
{
    memcpy(read_buffer,mem_nvm.uuid, sizeof(mem_nvm.uuid));
    return mem_nvm.retVal_getUuid;
}

/**@}*/

/* vim: set ai cin et sw=4 ts=4 : */
