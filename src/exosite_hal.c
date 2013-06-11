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
#define GS_RECV_OFFSET 1
static uint8_t cid = 0xff;

static int32_t exo_recv_index = -1; // first socket buffer byte was GainSpan command byte

// used to determine if NVM is available for use
static uint8_t hasNVM = 0;

// local functions

// externs
//extern void DisplayLCD(uint8_t, const uint8_t *);
//extern char *itoa(int n, char *s, int b);

// global variables


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
void exoHAL_SocketClose(int32_t socket)
{
    if(socket == (int32_t)cid)
    {
        if (ifaceType == IF_NOVATEL)
        {
            //AtModem_SocketClose(cid);
            cid = 0xff;
            // 1 second delay required after disconnect
            //MSTimerDelay(1000);
            /* open novatel socket */
        }
        else
        {
            //AtLibGs_Close(cid);
            cid = 0xff;
            exo_recv_index = -1;
        }
    }
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

    unsigned char server[META_SERVER_SIZE];
    char serverip[20];

    if (cid != 0xff)
        return -1;

    exosite_meta_read((char *)server, META_SERVER_SIZE, META_SERVER);

    // convert to string IP
    sprintf(  serverip,
        "%hhu.%hhu.%hhu.%hhu",
        server[0],
        server[1],
        server[2],
        server[3]);

    if (ifaceType == IF_NOVATEL)
    {
        //if(AtModem_SocketOpen(serverip, server[5], &cid) == -1)
        //{
        //    exoHAL_HandleError(EXO_ERROR_CONNECT);
        //    return -1;
        //}
        //else
        //{
        //    // delay required after socket opened and before writing to it
        //    MSTimerDelay(500);
        //}
    }
    else
    {
        //AtLibGs_TCPClientStart(serverip, server[5], &cid);
    }

    return (int32_t)cid;
}


/*****************************************************************************
*
*  exoHAL_ServerConnect
*
*  \param  None
*
*  \return socket - socket handle
*
*  \brief  The function opens a TCP socket
*
*****************************************************************************/
int32_t exoHAL_ServerConnect(int32_t sock)
{
    //TODO - use DNS or check m2.exosite.com/ip to check for updates
    if( sock == (int32_t)cid)
        return (int32_t)cid;
    else
        return -1;
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
uint8_t exoHAL_SocketSend(int32_t socket, char * buffer, uint8_t len)
{
    //App_PrepareIncomingData();
    if(socket == (int32_t)cid)
    {
        //if (ifaceType == IF_NOVATEL)
        //{
        //    Modem_Socket_Send(cid, buffer, len);
        //}
        //else
        //{
        //    AtLibGs_SendTCPData(cid, (char *)buffer, len);
        //}
    }
    else
        len = 0;

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
uint16_t exoHAL_SocketRecv(int32_t socket, char * buffer, uint8_t len)
{
    if (socket == (int32_t)cid)
    {
        int rec_len = 0, rxbufsize = 0;
        //ATLIBGS_MSG_ID_E rxMsgId = ATLIBGS_MSG_ID_NONE;

        //if (exo_recv_index == -1)
        //{
        //    if (ifaceType == IF_NOVATEL)
        //    {
        //        // read incoming data for 10 seconds,
        //        rxMsgId = (ATLIBGS_MSG_ID_E)AtModem_ReadLineTimeOut( 10000, buffer, len);
        //        // novatel doesn't need the rest of this stuff
        //        return rxMsgId;
        //    }
        //    else
        //    {
        //        rxMsgId = AtLibGs_ReceiveDataHandle(3000);
        //    }
        //    if (ATLIBGS_MSG_ID_DATA_RX != rxMsgId || G_receivedCount <= GS_RECV_OFFSET)
        //        return 0;
        //    exo_recv_index = GS_RECV_OFFSET;
        //}
        //rxbufsize = G_receivedCount - exo_recv_index;

        //rec_len = len <= rxbufsize ? len : rxbufsize;
        //memcpy(buffer, &G_received[exo_recv_index], rec_len);
        //exo_recv_index += rec_len;

        //if(exo_recv_index == G_receivedCount)
        //{
        //    exo_recv_index = -1;
        //}

        //return rec_len;
    }

    return 0;
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
    //    exoHAL_ShowErrorMessage("Not Activatd",sizeof("Not Activatd"));
    //    while(1);
    //    
    //}

    return;
}


