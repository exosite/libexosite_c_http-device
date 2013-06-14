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
#include "exosite.h"
#include "exosite_hal.h"
#include "exosite_meta.h"
#include <string.h>
#include <stdio.h>


// local variables


// used to determine if NVM is available for use
static uint8_t hasNVM = 0;

static uint8_t isSocketOpen = 0;/*!< 0 if closed 1 if open */

// local functions

// externs


// global variables

// global functions
// Convert an integer into a char
char* Itoa(int value, char* str, int radix) {
    static char dig[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";
    int n = 0, neg = 0;
    unsigned int v;
    char* p, *q;
    char c;
    if (radix == 10 && value < 0) {
        value = -value;
        neg = 1;
    }
    v = value;
    do {
        str[n++] = dig[v%radix];
        v /= radix;
    } while (v);
    if (neg)
        str[n++] = '-';
    str[n] = '\0';
    for (p = str, q = p + n/2; p != q; ++p, --q)
        c = *p, *p = *q, *q = c;
    return str;
}

// simple memcpy 
void* memcpy(void* dest, const void* src, size_t count) {
    char* dst8 = (char*)dest;
    char* src8 = (char*)src;

    while (count--) {
        *dst8++ = *src8++;
    }
    return dest;
}

UUIDInterfaceTypes ifaceType = IF_NONE;


/*!
* \brief  Selects the interface that this HAL will use
*
*	This is an optional method and can be used if there are
*	multiple interfaces into your hardware and you want the
*	option to have multiple interfaces in the same source file
*
*
* \param[in] type The interface type to use
*
*/
void exoHAL_SetIface(UUIDInterfaceTypes type)
{
    ifaceType = type;
}


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


    return;
}


/*!
* \brief  Writes data to NVM
* 
*	
*
* \param[in] buffer Buffer of data to write to NVM 
* \param[in] len Length of data to write to NVM. 
* \param[in] offset offset, from base meta, to write buffer to 
*
*
*/
void exoHAL_WriteMetaItem(char * buffer, uint8_t len, int32_t offset)
{

    return;
}


/*!
* \brief  Reads data from NVM
* 
*	
*
* \param[in] buffer Buffer to write data, from NVM, into 
* \param[in] len Length of data to read from NVM. 
* \param[in] offset offset, from base meta, to read from
*
*
*/
void exoHAL_ReadMetaItem(char * buffer, uint8_t len, int32_t offset)
{


    return;
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
void exoHAL_SocketClose()
{
    isSocketOpen = 0;
    return;
}


/*****************************************************************************
*
*  exoHAL_SocketOpenTCP
*
*  \param  None
*
*  \return -1: failure; else 0
*
*  \brief  The function opens a TCP socket
*
*****************************************************************************/
int32_t exoHAL_SocketOpenTCP()
{
        isSocketOpen = 1
        return 0;
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
uint8_t exoHAL_SocketSend( char * buffer, uint8_t len)
{
  
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
uint16_t exoHAL_SocketRecv(int32_t socket, char * buffer, uint8_t len)
{
  return 1;
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
        //DisplayLCD(LCD_LINE6, " *Exosite* ");
        //DisplayLCD(LCD_LINE7, " Write err ");
        //DisplayLCD(LCD_LINE8, "           ");
        break;
    case EXO_ERROR_CONNECT:
        //DisplayLCD(LCD_LINE6, " *Exosite* ");
        //DisplayLCD(LCD_LINE7, "Connect err");
        //DisplayLCD(LCD_LINE8, "           ");
        break;
    default:
        //DisplayLCD(LCD_LINE6, " *Exosite* ");
        //DisplayLCD(LCD_LINE7, " Unknow err");
        //DisplayLCD(LCD_LINE8, "           ");
        break;
    }

    return;
}


/*****************************************************************************
*
*  exoHAL_ShowUIMessage
*
*  \param  code - UI code for message to display;
*
*  \return None
*
*  \brief  Displays message in a platform specific way
*
*****************************************************************************/
void exoHAL_ShowUIMessage(ExositeUICodes code)
{
    switch (code)
    {
    case EXO_SERVER_CONNECTED:
        //DisplayLCD(LCD_LINE6, "  Exosite  ");
        //DisplayLCD(LCD_LINE7, " Connected ");
        break;
    case EXO_CLIENT_RW:
        //DisplayLCD(LCD_LINE6, "  Exosite  ");
        //DisplayLCD(LCD_LINE7, " Read/Write");
        break;
    default:
        break;
    }

    return;
}

// displays an error message, limited to 12 chars
void exoHAL_ShowErrorMessage(char * errMsg, uint8_t length)
{
    //DisplayLCD(LCD_LINE7, "Error:");
    //DisplayLCD(LCD_LINE8, errMsg);
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
    //    exoHAL_ShowErrorMessage("Not Activated",sizeof("Not Activated"));
    //    while(1);
    //    
    //}

    return;
}


