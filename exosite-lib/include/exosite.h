/*****************************************************************************
*
*  exosite.h - Exosite library interface header
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

#ifndef EXOSITE_H
#define EXOSITE_H

#include <stdint.h>


// DEFINES
#define CIK_LENGTH                              40
#define MAX_UUID_LENGTH                         40
#define MAX_VENDOR_LENGTH                       20
#define MAX_MODEL_LENGTH                        20

/*!< This defines the maximum size that a string can be for sending data
to Exosite.  It is used to prevent exosite_strlen from overrunning.
If you are have a need to increase string length, you can freely adjust
this number up to uint16_t*/

/*!< This defines the size of the rx buffer in exosite.c.  This buffer is used
to place incoming data from the modem in.*/
#define RX_BUFFER_SIZE                         512


// ENUMS
typedef enum EXOSITE_DEVICE_ACTIVATION_STATE_tag
{
    CONNECTION_ERROR,       /*!< Error in connecting to Exosite */
    VALID_CIK,              /*!< CIK is valid  */
    DEVICE_NOT_ENABLED,     /*!< Device has not been enabled */
    R_W_ERROR,               /*!< R/W error */
    UNABLE_TO_MAKE_REQUEST  /*!< string was too long */
}EXOSITE_DEVICE_ACTIVATION_STATE;



// PUBLIC FUNCTIONS
EXOSITE_DEVICE_ACTIVATION_STATE exosite_activate();
EXOSITE_DEVICE_ACTIVATION_STATE exosite_init(const char *vendor, const char *model);

uint8_t exosite_write(const char * writeData, uint16_t length);
uint8_t exosite_read(const char * alias, char * readResponse, uint16_t buflen, uint16_t * length);

uint8_t exosite_isCIKValid(char cik[CIK_LENGTH]);
void exosite_setCIK(char * pCIK);

uint8_t exosite_resetCik();

void exosite_getCIK(char * pCIK);
#endif



