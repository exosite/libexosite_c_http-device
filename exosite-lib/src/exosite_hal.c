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




// local functions

// externs
//extern void DisplayLCD(uint8_t, const uint8_t *);
//extern char *itoa(int n, char *s, int b);

// global variables
#ifndef TESTING


// simple memcpy
void*memcpy(void* dest, const void* src, uint32_t count)
{
    char* dst8 = (char*)dest;
    char* src8 = (char*)src;

    while (count--)
    {
        *dst8++ = *src8++;
    }
    return dest;
}

uint16_t strlen(const char *s)
{
    uint16_t retVal = 0;
    while (s[retVal])
    {
        retVal++;
    }
    return retVal;
}

#endif

/*!< Used to reserve memory to emulate NVM during unit testing */
struct UnitTest_meta_storage
{
    char cik[40];
    char uuid[20];
    char vendor[15];
    char model[15];
    char server[40];
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


UUIDInterfaceTypes ifaceType = IF_NONE;


/*!
 * \brief  Selects the interface that this HAL will use
 *
 *	This is an optional method and can be used if there are
 *	multiple interfaces into your hardware and you want the
 *	option to have multiple interfaces in the same source file
 *
 *
 * \param[in] type The iterface type to use
 *
 */
void exoHAL_SetIface(UUIDInterfaceTypes type)
{
    ifaceType = type;
}

/*****************************************************************************
*
*  exoHAL_ReadHWMAC
*
*  \param  Interface Number (1 - WiFi), buffer to return hexadecimal MAC
*
*  \return 0 if failure; len of UUID if success;
*
*  \brief  Reads the MAC address from the hardware
*
*****************************************************************************/

/*!
 * \brief  Retrieves UUID from device
 *
 *	This function retrieves a unique ID from your device.  This is typically
 *	the MEID of a cell modem, MAC address of a network card, or serial number
 *	of the device.
 *
 *
 *
 *
 *
 * \param[in] UUID_buf Buffer to put the devices UUID into.
 *
 *  \return  if failed to retrieve UUID, else 0
 *
 */
uint8_t exoHAL_ReadUUID( char * UUID_buf)
{
    uint8_t retVal = 1;

    // Get Value of UUID and place into UUID_buf

    // if successful, set retVal to 0.


    return retVal;
}


/*****************************************************************************
*
* exoHAL_EnableNVMeta
*
*  \param  None
*
*  \return None
*
*  \brief  Enables meta non-volatile memory, if any
*
*****************************************************************************/
/*!
 * \brief  Performs any initialization of your NVM and notifies the HAL that
 *			NVM is available
 *
 */
void exoHAL_EnableMeta(void)
{
    return;
}



/*!
 * \brief  Fills NVM with zeros
 *
 *
 *
 *
 */
void exoHAL_EraseMeta(void)
{
    mem.cik = "";
    mem.model = "";
    mem.server = "";
    mem.uuid = "";
    mem.vendor = "";
}



/*****************************************************************************
*
*  exoHAL_SocketClose
*
*  \param  socket - socket handle
*
*  \return None
*
*  \brief  The function closes a socket
*
*****************************************************************************/
void exoHAL_SocketClose(int32_t socket)
{

    return;
}


/*****************************************************************************
*
*  exoHAL_SocketOpenTCP
*
*  \param  None
*
*  \return -1: failure; Other: socket handle
*
*  \brief  The function opens a TCP socket
*
*****************************************************************************/
int32_t exoHAL_SocketOpenTCP()
{

    return 1;
}




/*****************************************************************************
*
*  exoHAL_SocketSend
*
*  \param  socket - socket handle; buffer - string buffer containing info to
*          send; len - size of string in bytes;
*
*  \return Number of bytes sent
*
*  \brief  Sends data out the network interface
*
*****************************************************************************/
uint8_t exoHAL_SocketSend( const char * buffer, uint8_t len)
{
    return len;
}


/*****************************************************************************
*
*  exoHAL_SocketRecv
*
*  \param  socket - socket handle; buffer - string buffer to put info we
*          receive; len - size of buffer in bytes;
*
*  \return Number of bytes received
*
*  \brief  Sends data out the network interface
*
*****************************************************************************/
uint16_t exoHAL_SocketRecv( char * buffer, uint8_t len)
{


    return len;
}


/*****************************************************************************
*
*  exoHAL_HandleError
*
*  \param  code - error code;
*
*  \return None
*
*  \brief  Handles errors in platform-specific way
*
*****************************************************************************/
void exoHAL_HandleError(ExositeErrorCodes code)
{
    switch (code)
    {
    case EXO_ERROR_WRITE:
        //do something
        break;
    case EXO_ERROR_CONNECT:
        //do something
        break;
    default:
        //do something
        break;
    }

    return;
}




/*****************************************************************************
*
*  exoHAL_MSDelay
*
*  \param  delay - milliseconds to delay
*
*  \return None
*
*  \brief  Delays for specified milliseconds
*
*****************************************************************************/
void exoHAL_MSDelay(uint16_t delay)
{
    //MSTimerDelay(delay);

    return;
}


// perform modem initialization in here
void exoHAL_initModem()
{


    //AtModem_Init();

    // if modem isn't activated, tell user.
    //if(!Novatel_isModemActivate())
    //{
    //    exoHAL_ShowErrorMessage("Not Activatd",sizeof("Not Activatd"));
    //    while(1);
    //
    //}

    return;
}

/*****************************************************************************
*
*  exosite_meta_init
*
*  \param  None
*
*  \return None
*
*  \brief  Does whatever we need to do to initialize the NV meta structure
*
*****************************************************************************/
void exosite_meta_init(void)
{
    // Perform and NVM/hardware initialization here

    // This is intended to get NVM hardware ready to read/write, not
    // actually do any reading or writing.

    return;
}




/*****************************************************************************
*
*  exosite_meta_write
*
*  \param  write_buffer - string buffer containing info to write to meta;
*          srcBytes - size of string in bytes; element - item from
*          MetaElements enum.
*
*  \return None
*
*  \brief  Writes specific meta information to meta memory.
*
*****************************************************************************/
void exoHAL_meta_write(const char * write_buffer, uint16_t srcBytes, MetaDataTypes element)
{

    switch (element)
    {
    case EXO_META_CIK:
        memcpy(mem_nvm.cik, write_buffer,srcBytes);
        break;
    case EXO_META_MODEL:
        memcpy(mem_nvm.model, write_buffer,srcBytes);
        break;
    case EXO_META_VENDOR:
        memcpy(mem_nvm.vendor, write_buffer,srcBytes);
        break;
    case EXO_META_SERVER:
        memcpy(mem_nvm.server, write_buffer,srcBytes);
        break;
    case EXO_META_UUID:
        memcpy(mem_nvm.uuid, write_buffer,srcBytes);
        break;
    }

    return;
}


/*****************************************************************************
*
*  exosite_meta_read
*
*  \param  read_buffer - string buffer to receive element data; destBytes -
*          size of buffer in bytes; element - item from MetaElements enum.
*
*  \return number of bytes read
*
*  \brief  Writes specific meta information to meta memory.
*
*****************************************************************************/
void exoHAL_meta_read(char * read_buffer, MetaDataTypes element)
{
    switch (element)
    {
    case EXO_META_CIK:
        memcpy( read_buffer,mem_nvm.cik,sizeof(mem_nvm.cik));
        break;
    case EXO_META_MODEL:
        memcpy(read_buffer,mem_nvm.model, sizeof(mem_nvm.model));
        break;
    case EXO_META_VENDOR:
        memcpy(read_buffer,mem_nvm.vendor, sizeof(mem_nvm.vendor));
        break;
    case EXO_META_SERVER:
        memcpy(read_buffer,mem_nvm.server, sizeof(mem_nvm.server));
        break;
    case EXO_META_UUID:
        memcpy(read_buffer,mem_nvm.uuid, sizeof(mem_nvm.uuid));
        break;
    }

    return;
}

char*exoHAL_itoa(int value, char* str, int radix)
{
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
    for (p = str, q = p + n/2; p != q; ++p, --q)
        c = *p, *p = *q, *q = c;
    return str;
}

