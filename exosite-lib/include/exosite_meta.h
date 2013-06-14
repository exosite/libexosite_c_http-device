/*****************************************************************************
*
*  exosite_meta.h - Meta informatio header
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

#ifndef EXOSITE_META_H
#define EXOSITE_META_H

#include <stdint.h>

typedef enum
{
    EXO_META_CIK,    /*!< CIK of this individual Device*/
    EXO_META_UUID,   /*!< This devices UUID (e.g. serial #/MAC/MEID) */
    EXO_META_VENDOR, /*!< The vendor of this device as known to Exosite */
    EXO_META_MODEL,  /*!< The model of this device as known to Exosite */
    EXO_META_SERVER  /*!< The Exosite server URL */

}MetaDataTypes; /*!< Different types of data that are stored in NVM */


// functions for export
void exoHAL_meta_init(void);
void exoHAL_meta_write(char * write_buffer, uint16_t srcBytes, MetaDataTypes element);
void exoHAL_meta_read(char * read_buffer, uint16_t destBytes, MetaDataTypes element);

#endif



