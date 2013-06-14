/*****************************************************************************
*
*  exosite_meta.c - Exosite meta information handler.
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
#include "exosite_meta.h"
#include "exosite_hal.h"
//#include <string.h>



// external functions
// externs
// local functions
// exported functions
// local defines
// globals

/*!< Used to mock out NVM during unit testing */
struct UnitTest_meta_storage
{
   char cik[40];
   char uuid[20];
   char vendor[15];
   char model[15];
   char server[40];
};

struct UnitTest_meta_storage nvm;
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
void exosite_meta_write(char * write_buffer, uint16_t srcBytes, MetaTypes element)
{

    switch (element)
    {
    case EXO_META_CIK:
            memcpy(nvm.cik, write_buffer,srcBytes);
        break;
    case EXO_META_MODEL:
            memcpy(nvm.cik, write_buffer,srcBytes);
        break;
    case EXO_META_VENDOR:
            memcpy(nvm.cik, write_buffer,srcBytes);
        break;
    case EXO_META_SERVER:
            memcpy(nvm.cik, write_buffer,srcBytes);
        break;
    case EXO_META_UUID:
            memcpy(nvm.cik, write_buffer,srcBytes);
        break;
    case META_NONE:
    default:
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
*  \return None
*
*  \brief  Writes specific meta information to meta memory.
*
*****************************************************************************/
void exosite_meta_read(char * read_buffer, uint16_t destBytes, int32_t element)
{
    exosite_meta * meta_info = 0;

    switch (element)
    {
    case META_CIK:
        if (destBytes < META_CIK_SIZE)
            return;
        exoHAL_ReadMetaItem(read_buffer, META_CIK_SIZE, (int)meta_info->cik); //read CIK
        break;
    case META_SERVER:
        if (destBytes < META_SERVER_SIZE)
            return;
        exoHAL_ReadMetaItem(read_buffer, META_SERVER_SIZE, (int)meta_info->server); //read server IP
        break;
    case META_MARK:
        if (destBytes < META_MARK_SIZE)
            return;
        exoHAL_ReadMetaItem(read_buffer, META_MARK_SIZE, (int)meta_info->mark); //read exosite mark
        break;
    case META_UUID:
        if (destBytes < META_UUID_SIZE)
            return;
        exoHAL_ReadMetaItem(read_buffer, META_UUID_SIZE, (int)meta_info->uuid); //read provisioning UUID
        break;
    case META_MFR:
        if (destBytes < META_MFR_SIZE)
            return;
        exoHAL_ReadMetaItem(read_buffer, META_MFR_SIZE, (int)meta_info->mfr); //read manufacturing info
        break;
    case META_NONE:
    default:
        break;
    }

    return;
}



