/*****************************************************************************
*
*  exosite_async.h - Exosite library interface header
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

#ifndef __EXOSITE_ASYNC_H__
#define __EXOSITE_ASYNC_H__

// DEFINES
#define CIK_LENGTH                              40
#define MAX_UUID_LENGTH                         40
#define MAX_VENDOR_LENGTH                       20

/*!< This defines the maximum size that a string can be for sending data
   to Exosite.  It is used to prevent exosite_strlen from overrunning.
   If you are have a need to increase string length, you can freely adjust
   this number up to uint16_t*/

/******************************************************************************/

/* State machine for sending HTTP requests to Exosite HTTP-Device-API.
 */

enum exoHttp_req_e {
    exoHttp_req_method_url = 0,
    exoHttp_req_query,
    exoHttp_req_http_version,
    exoHttp_req_host,
    exoHttp_req_cik,
    exoHttp_req_content,
    exoHttp_req_accept,
    exoHttp_req_timeout,
    exoHttp_req_modified_since,
    exoHttp_req_content_length,
    exoHttp_req_start_body,
    exoHttp_req_body,
    exoHttp_req_complete
};
struct exoHttp_req_s {
    enum exoHttp_req_e step;
    size_t content_length;
    int is_post:1; //!< POST or GET?
    int include_cik:1; //!< Iinclude CIK header.
    int is_activate:1; //!< Don't use body, compute activation body instead.
    const char * method_url_path;
    const char * query;
    const char * body;
    const char * modified_since; //!< For LongPoll, start time for waiting
    uint32_t request_timeout; //!< For LongPoll, how many milliseconds to wait. (max 300,000)
};
typedef struct exoHttp_req_s exoHttp_req_t;

enum exoHttp_rpl_e {
    exoHttp_rpl_looking_for_status = 0,
    exoHttp_rpl_read_status,
    exoHttp_rpl_status_looking_for_cr,
    exoHttp_rpl_status_looking_for_lf,
    exoHttp_rpl_header_start,
    exoHttp_rpl_header_mark_name,
    exoHttp_rpl_header_looking_for_sep,
    exoHttp_rpl_header_mark_value,
    exoHttp_rpl_header_looking_for_cr,
    exoHttp_rpl_header_looking_for_lf,

    exoHttp_rpl_looking_for_lf_start_body,
    exoHttp_rpl_body,
    exoHttp_rpl_complete,
    exoHttp_rpl_error
};
struct exoHttp_rpl_s {
    enum exoHttp_rpl_e step;
    uint16_t statusCode; //!> HTTP status code
    size_t content_length; //!> Value of Content-Length header, if it exists.

};
typedef struct exoHttp_rpl_s exoHttp_rpl_t;

/******************************************************************************/
enum Exosite_state_e {
    Exosite_State_initialized = 0,
    Exosite_State_pal_starting,
    Exosite_State_idle,

    Exosite_State_activate,
    Exosite_State_write,
    Exosite_State_read,
    Exosite_State_hybrid,
    Exosite_State_longpoll,
    Exosite_State_timestamp,

};
enum Exosite_Stage_e {
    Exosite_Stage_idle = 0,
    Exosite_Stage_connecting,
    Exosite_Stage_sending,
    Exosite_Stage_waiting,
    Exosite_Stage_recving,
    Exosite_Stage_recving_body,
    Exosite_Stage_closing,
};
typedef struct Exosite_ops_s Exosite_ops_t;
typedef struct Exosite_state_s Exosite_state_t;
typedef int (*exo_status_cb) (Exosite_state_t *state, int status);
typedef int (*exo_data_cb) (Exosite_state_t *state, const char *data, size_t len);

struct Exosite_ops_s {
    exo_status_cb on_start_complete;
    exo_status_cb on_write_complete;
    exo_status_cb on_read_begin;
    exo_data_cb   on_read_raw;
#if 0
    exo_data_cb   on_read_alias;
    exo_data_cb   on_read_value;
#endif
    exo_status_cb on_read_complete;
    exo_data_cb   on_timestamp_complete;
};

struct Exosite_state_s {
    // Private
    enum Exosite_state_e state;
    enum Exosite_Stage_e stage;

    char projectid[MAX_VENDOR_LENGTH+1];
    char cik[CIK_LENGTH+1];
    char uuid[MAX_UUID_LENGTH+1];

    // union these two?
    exoHttp_req_t http_req;
    exoHttp_rpl_t http_rpl;

    char workbuf[80]; //!> Working buffer to build up sends and pull-in receives
    int wb_offset; //!> If we need to recv again to get enough data.

    int statusCode; //!>  status code

    // Public
    Exosite_ops_t ops;
    exoPal_state_t *exoPal;

    void *context; //!< Use specific context data.
};


void exosite_init(Exosite_state_t *state);
/*
 * - call init
 * - set pal pointer
 * - set ops.callbacks
 * - optionally set context.
 * - call start
 */

int exosite_start(Exosite_state_t *state);

int exosite_write(Exosite_state_t *state, const char *aliasesAndValues);
int exosite_read(Exosite_state_t *state, const char *aliases);
int exosite_hybrid(Exosite_state_t *state, const char *writeAliasesAndValues, const char *readAliases);
int exosite_longpoll(Exosite_state_t *state, const char *aliases, const char *timestamp, uint32_t timeout);
int exosite_timestamp(Exosite_state_t *state);


#ifdef EXO_ASSERT_CHECKS
#define EXO_ASSERT(x) assert(x)
#else
#define EXO_ASSERT(...)
#endif
#endif /*__EXOSITE_ASYNC_H__*/
