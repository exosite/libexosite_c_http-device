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




// ENUMS
/*!
 * Tracks the current state of the device on the Exosite Platform.
 */
typedef enum EXOSITE_DEVICE_STATE_tag
{
    EXO_STATE_CONNECTION_ERROR,       /*!< Error in connecting to Exosite */
    EXO_STATE_VALID_CIK,              /*!< CIK is valid  */
    EXO_STATE_DEVICE_NOT_ENABLED,     /*!< Device has not been enabled */
    EXO_STATE_R_W_ERROR,              /*!< R/W error */
    EXO_STATE_UNABLE_TO_MAKE_REQUEST, /*!< string was too long */
    EXO_STATE_INIT_COMPLETE,          /*!< Initialization successfully completed. */
    EXO_STATE_NOT_COMPLETE,           /*!< Device has not yet completed initialization. */
    EXO_STATE_INIT_ERROR,             /*!< An error occured in exosite_init(). */
    EXO_STATE_NO_RESPONSE             /*!< No response was received from modem */
}EXO_STATE;



// PUBLIC FUNCTIONS
EXO_STATE exosite_activate();
EXO_STATE exosite_init(const char *vendor, const char *model);

int32_t exosite_write(const char * writeData, uint16_t length);
int32_t exosite_read(const char * alias, char * readResponse, uint16_t buflen, uint16_t * responseSize);
int32_t exosite_readSingle(const char * alias, char * readResponse, uint16_t buflen, uint16_t * length);

int8_t exosite_getTimestamp(int32_t * timestamp);
#if 0
int32_t exosite_rawRpcRequest(const char * requestBody, uint16_t requestLength, char * responseBuffer, int32_t responseBufferLength);
#endif

// internal.
int32_t exosite_getBody(char *response, char **bodyStart, uint16_t *bodyLength);
uint8_t exosite_isCIKValid(char cik[CIK_LENGTH]);


void exosite_setCIK(char * pCIK);
uint8_t exosite_resetCik();
void exosite_getCIK(char * pCIK);
#endif


#ifndef __G__G__
#define __G__G__

enum Exosite_state_e {
    Exosite_State_uninitialized = 0,
    Exosite_State_idle,

    Exosite_State_dns_lookup,

    Exosite_State_activate_sending,
    Exosite_State_activate_waiting,
    Exosite_State_activate_recving,

    Exosite_State_write_sending,
    Exosite_State_write_waiting,
    Exosite_State_write_recving,

    Exosite_State_read_sending,
    Exosite_State_read_waiting,
    Exosite_State_read_recving,

    Exosite_State_hybrid_sending,
    Exosite_State_hybrid_waiting,
    Exosite_State_hybrid_recving,

    Exosite_State_longpoll_sending,
    Exosite_State_longpoll_waiting,
    Exosite_State_longpoll_recving,

    Exosite_State_timestamp_sending,
    Exosite_State_timestamp_waiting,
    Exosite_State_timestamp_recving,

};
typedef struct Exosite_state_s {
    // Private
    enum Exosite_state_e state;

    char projectid[MAX_VENDOR_LENGTH];
    char cik[CIK_LENGTH];
    char uuid[MAX_UUID_LENGTH];

    // Public
    Exosite_ops_t ops;

    void *context; //!< Use specific context data.
} Exosite_state_t;

typedef int (*exo_status_cb) (Exosite_state_t *state, int status);
typedef int (*exo_data_cb) (Exosite_state_t *state, const char *data, size_t len);

typedef struct Exosite_ops_s {
    exo_status_cb on_start_complete;
    exo_status_cb on_write_complete;
    exo_status_cb on_read_begin;
    exo_data_cb   on_read_raw;
    exo_data_cb   on_read_alias;
    exo_data_cb   on_read_value;
    exo_status_cb on_read_complete;
    exo_status_cb on_timestamp_complete;
} Exosite_ops_t;


int exosite_init(Exosite_state_t *state);

int exosite_start(Exosite_state_t *state);

int exosite_write(Exosite_state_t *state, const char *aliasesAndValues);
int exosite_read(Exosite_state_t *state, const char *aliases);
int exosite_hybrid(Exosite_state_t *state, const char *writeAliasesAndValues, const char *readAliases);
int exosite_longpoll(Exosite_state_t *state, const char *aliases, const char *timestamp, uint32_t timeout);
int exosite_timestamp(Exosite_state_t *state);

#endif /*__G__G__*/

/* vim: set ai cin et sw=4 ts=4 : */
