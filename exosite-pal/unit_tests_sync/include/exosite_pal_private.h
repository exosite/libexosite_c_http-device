/*****************************************************************************
*
*  exosite_hal.h - Common header for Exosite hardware adapation layer
*  Copyright (C) 2012-2017 Exosite LLC
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

#ifndef EXOSITE_PAL_PRIVATE_H
#define EXOSITE_PAL_PRIVATE_H
#include <stdint.h>



// defines
typedef enum
{
    EXO_ERROR_WRITE,
    EXO_ERROR_READ,
    EXO_ERROR_CONNECT,
    EXO_ERROR_UNKNOWN,
    EXO_ERROR_END
}ExositeErrorCodes;

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
    uint8_t retVal_tcpSocketClose;
    uint8_t retVal_tcpSocketOpen;
    uint8_t retVal_socketRead;
    uint8_t retVal_socketWrite;

};

void * getUnitTestStorageStruct();



#endif
/* vim: set ai cin et sw=4 ts=4 : */
