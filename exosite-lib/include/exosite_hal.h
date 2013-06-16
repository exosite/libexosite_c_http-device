/*****************************************************************************
*
*  exosite_hal.h - Common header for Exosite hardware adapation layer
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

#ifndef EXOSITE_HAL_H
#define EXOSITE_HAL_H

#include "exosite.h"



// defines
typedef enum 
{
    EXO_ERROR_WRITE,
    EXO_ERROR_READ,
    EXO_ERROR_CONNECT,
    EXO_ERROR_UNKNOWN,
    EXO_ERROR_END
}ExositeErrorCodes;

typedef enum 
{
    EXO_SERVER_CONNECTED,
    EXO_CLIENT_RW,
    EXO_UI_END
}ExositeUICodes;

// defines
typedef enum
{
	IF_WIFI,	/*!<  */
	IF_ENET,
	IF_FILE,
	IF_HDD,
	IF_I2C,
	IF_GPRS,
	IF_NOVATEL,
	IF_NONE
}UUIDInterfaceTypes;

// functions for export



typedef enum
{
    EXO_META_CIK,    /*!< CIK of this individual Device*/
    EXO_META_UUID,   /*!< This devices UUID (e.g. serial #/MAC/MEID) */
    EXO_META_VENDOR, /*!< The vendor of this device as known to Exosite */
    EXO_META_MODEL,  /*!< The model of this device as known to Exosite */
    EXO_META_SERVER  /*!< The Exosite server URL */

}MetaDataTypes; /*!< Different types of data that are stored in NVM */





void exoHAL_meta_write(char * write_buffer, uint16_t srcBytes, MetaDataTypes element);
void exoHAL_meta_read(char * read_buffer, MetaDataTypes element);
uint8_t exoHAL_ReadUUID( char * UUID_buf);
void exoHAL_EnableMeta(void);
void exoHAL_EraseMeta(void);
void exoHAL_SocketClose(int32_t socket);
int32_t exoHAL_SocketOpenTCP(void);
uint8_t exoHAL_SocketSend(char * buffer, uint8_t len);
uint16_t exoHAL_SocketRecv(char * buffer, uint8_t len);
void exoHAL_HandleError(ExositeErrorCodes code);
void exoHAL_ShowUIMessage(ExositeUICodes code);
void exoHAL_MSDelay(uint16_t delay);
void exoHAL_SetIface(UUIDInterfaceTypes type);
void exoHAL_initModem();
void exoHAL_ShowErrorMessage(char * errMsg, uint8_t length);

void exosite_meta_init(void);

#ifndef TESTING
uint16_t strlen(const char *s);
void* memcpy(void* dest, const void* src, uint32_t count);
char* Itoa(int value, char* str, int radix);
#endif

#endif


