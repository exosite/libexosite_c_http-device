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
#include <stdlib.h>
#include "exosite_pal_async.h"

#define CIK_LENGTH        (40) //!< Length of a CIK
#define MAX_UUID_LENGTH   (40) //!< Maximum size of an idenifier
#define MAX_VENDOR_LENGTH (20) //!< Maximum size of a Vendor name
#define MAX_MODEL_LENGTH  (20) //!< Maximum size of a Model name

/******************************************************************************/

enum exoHttp_req_e {
    exoHttp_req_method_url = 0,
    exoHttp_req_query,
    exoHttp_req_http_version,
    exoHttp_req_host,
    exoHttp_req_agent,
    exoHttp_req_connection,
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
/** State machine for sending HTTP requests to Exosite HTTP-Device-API.
 * http://docs.exosite.com/reference/products/device-api/http/
 */
struct exoHttp_req_s {
    enum exoHttp_req_e step; //!< Where it is on sending data.
    size_t content_length; //!< how much data is in body.
    int is_post:1; //!< POST or GET?
    int include_cik:1; //!< Include CIK header.
    int is_activate:1; //!< Don't use body, compute activation body instead.
    const char * method_url_path; //!< URL Path for request
    const char * query; //!< query params, usually the aliases to read
    const char * body; //!< POST body, usually aliases and data to write
    const char * modified_since; //!< For LongPoll, start time for waiting
    uint32_t request_timeout; //!< For LongPoll, how many milliseconds to wait. (max 300,000)
};
typedef struct exoHttp_req_s exoHttp_req_t;

/******************************************************************************/
enum exoHttp_rpl_e {
    exoHttp_rpl_looking_for_status = 0,
    exoHttp_rpl_read_status,
    exoHttp_rpl_status_looking_for_cr,
    exoHttp_rpl_status_looking_for_lf,
    exoHttp_rpl_header_start,
    exoHttp_rpl_header_looking_for_sep,
    exoHttp_rpl_header_mark_value,
    exoHttp_rpl_header_looking_for_cr,
    exoHttp_rpl_header_looking_for_lf,
    exoHttp_rpl_looking_for_lf_start_body,
    exoHttp_rpl_mark_body,
    exoHttp_rpl_body,
    exoHttp_rpl_complete,
    exoHttp_rpl_error
};
/**
 * State machine for receiving HTTP requests from Exosite HTTP-Device-API.
 * http://docs.exosite.com/reference/products/device-api/http/
 */
struct exoHttp_rpl_s {
    enum exoHttp_rpl_e step;
    int counter_content_length;

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

/** \brief Callback with a status value
 * \param[in] state The Exosite state structure
 * \param[in] status The status code. Usually a HTTP status code.
 */
typedef int (*exo_status_cb) (Exosite_state_t *state, int status);

/** \brief Callback with some data
 * \param[in] state The Exosite state structure
 * \param[in] data The data just read
 * \param[in] len How much data
 */
typedef int (*exo_data_cb) (Exosite_state_t *state, const char *data, size_t len);

/** \brief Callback with a timestamp
 * \param[in] state The Exosite state structure
 * \param[in] timestamp Seconds since the Unix EPOCH.
 */
typedef int (*exo_timestamp_cb) (Exosite_state_t *state, uint32_t timestamp);

/**
 * Various callbacks that can be handled.
 *
 * All of these can be set to NULL to ignore that callback.
 */
struct Exosite_ops_s {
    exo_status_cb    on_start_complete; //!< Callback when library is read for reads and writes.
    exo_status_cb    on_write_complete; //!< Callback when write is complete.
    exo_status_cb    on_read_begin; //!< Callback for the beginning of reading
    exo_data_cb      on_read_raw; //!< Callback with data from read.
#if 0
    // Maybe future feature.
    exo_data_cb      on_read_alias;
    exo_data_cb      on_read_value;
#endif
    exo_status_cb    on_read_complete; //!< Callback when all of the data has been read.
    exo_timestamp_cb on_timestamp_complete; //!< Callback with the timestamp from the server.
};

/**
 * The state of a conncetion to Exsoite.
 */
struct Exosite_state_s {
    // Private
    enum Exosite_state_e state; //!< Current state of the library
    enum Exosite_Stage_e stage; //!< Current stage within a state

    char projectid[MAX_VENDOR_LENGTH+1]; //!< The project.id (or Vendor)
    char modelid[MAX_MODEL_LENGTH+1]; //!< The model.id (or project.id)
    char cik[CIK_LENGTH+1]; //!< The assigned CIK token.
    char uuid[MAX_UUID_LENGTH+1]; //!< The Unique ID. (or serial number)

    /** Working place for handling the HTTP transaction. */
    union {
        exoHttp_req_t req; //!< Request state machine state
        exoHttp_rpl_t rpl; //!< Reply state machine state
    } http;

    char workbuf[80]; //!< Working buffer to build up sends and pull-in receives
    int wb_offset; //!< If we need to recv again to get enough data.

    // union these two?
    int statusCode; //!< Holding place status code
    uint32_t timestamp; //!< Holding place for the timestamp while it is received

    // Public
    Exosite_ops_t ops; //!< All of the callbacks
    exoPal_state_t *exoPal; //!< Plaform specific interface

    void *context; //!< User specific context data.
};


/**
 * \brief Initialize the memory
 * \param[out] state The Exosite state structure
 */
void exosite_init(Exosite_state_t *state);

/** \brief Start using the Exosite HTTP-Device API
 * \param[in,out] state The Exosite state structure
 * \retval 0 Startup ok, wait for callback.
 * \retval !0 Startup failed, callback will not get called.
 *
 * This call gets everything ready for doing reads and writes.  This includes doing
 * device activation if required.  It may also do DNS lookups.
 *
 * Once everything is ready the Exosite_ops_s::on_start_complete callback will be
 * called.
 *
 *
 * \code
 * Exosite_state_t exoLib;
 * exosite_init(&exoLib);
 * exoLib.exoPal = &exoPal;
 * exoLib.ops.on_start_complete = my_start_callback;
 * ret = exosite_start(&exoLib);
 * \endcode
 *
 */
int exosite_start(Exosite_state_t *state);

/** \brief Write to aliases on a device
 * \param[in,out] state The Exosite state structure
 * \param[in] aliasesAndValues Query style string of aliases and their values
 * \retval 0 Write ok, wait for callback.
 * \retval !0 Write failed, callback will not get called.
 *
 * Send off a write request. Once complete, the callback
 * Exosite_ops_s::on_write_complete will get called.
 *
 * \code
 * exoLib.ops.on_write_complete = my_write_callback;
 * ret = exosite_write(&exoLib, "temp=75&hum=30");
 * \endcode
 */
int exosite_write(Exosite_state_t *state, const char *aliasesAndValues);

/** \brief Read aliases
 * \param[in,out] state The Exosite state structure
 * \param[in] aliases The aliases to read seperated with '&'
 * \retval 0 Read ok, wait for callback.
 * \retval !0 Read failed, callback will not get called.
 *
 * Send off a read request. When complete, Exosite_ops_s::on_read_begin will be
 * called with the HTTP response code.  Then Exosite_ops_s::on_read_raw will be
 * called one or more times with the raw body data. Finally,
 * Exosite_ops_s::on_read_complete will get called when all of the data has been
 * fetched.
 *
 * The data returned to Exosite_ops_s::on_read_raw will be the form urlencoded
 * data.
 *
 *
 * \code
 * exoLib.ops.on_read_begin = my_read_begin;
 * exoLib.ops.on_read_raw = my_read_raw;
 * exoLib.ops.on_read_complete = my_read_complete;
 * ret = exosite_read(&exoLib, "temp&hum");
 * \endcode
 */
int exosite_read(Exosite_state_t *state, const char *aliases);

/** \brief Do a write followed immeaditally be a read in one call.
 * \param[in,out] state The Exosite state structure
 * \param[in] writeAliasesAndValues Query style string of aliases and their values
 * \param[in] readAliases The aliases to read seperated with '&'
 * \retval 0 Hybrid read/write ok, wait for callback.
 * \retval !0 Hybrid read/write failed, callback will not get called.
 *
 * Send off a hybrid read/write request. When complete,
 * Exosite_ops_s::on_read_begin will be called with the HTTP response code.  Then
 * Exosite_ops_s::on_read_raw will be called one or more times with the raw body
 * data. Finally, Exosite_ops_s::on_read_complete will get called when all of the
 * data has been fetched.
 *
 * The data returned to Exosite_ops_s::on_read_raw will be the form urlencoded
 * data.
 *
 *
 * \code
 * exoLib.ops.on_read_begin = my_read_begin;
 * exoLib.ops.on_read_raw = my_read_raw;
 * exoLib.ops.on_read_complete = my_read_complete;
 * ret = exosite_hybrid(&exoLib, "temp=75&hum=30", "led1&led2");
 * \endcode
 */
int exosite_hybrid(Exosite_state_t *state, const char *writeAliasesAndValues, const char *readAliases);

/** \brief Start a LongPoll for data that will cahnge in the future
 * \param[in,out] state The Exosite state structure
 * \param[in] alias The alias to wait on
 * \param[in] modified_since
 * \param[in] timeout Max time to wait in milliseconds. (MAX value is 300000)
 * \retval 0 LongPoll ok, wait for callback.
 * \retval !0 LongPoll failed, callback will not get called.
 *
 * Send off a LongPoll request. When complete, Exosite_ops_s::on_read_begin will be
 * called with the HTTP response code.  Then Exosite_ops_s::on_read_raw will be
 * called one or more times with the raw body data. Finally,
 * Exosite_ops_s::on_read_complete will get called when all of the data has been
 * fetched.
 *
 * This will wait, upto #timeout, before returning.  If nothing has changed, this
 * returns with a 304.
 *
 * The data returned to Exosite_ops_s::on_read_raw will be the form urlencoded data.
 *
 *
 * \code
 * exoLib.ops.on_read_begin = my_read_begin;
 * exoLib.ops.on_read_raw = my_read_raw;
 * exoLib.ops.on_read_complete = my_read_complete;
 * ret = exosite_longpoll(&exoLib, "temp&hum", NULL, 10000);
 * \endcode
 *
 */
int exosite_longpoll(Exosite_state_t *state, const char *alias, const char *modified_since, uint32_t timeout);

/** \brief Get current time
 * \param[in,out] state The Exosite state structure
 * \retval 0 Timestamp ok, wait for callback.
 * \retval !0 Timestamp failed, callback will not get called.
 *
 * Requests the current time from the servers.  When complete,
 * Exosite_ops_s::on_timestamp_complete will be called.
 *
 *
 * \code
 * exoLib.ops.on_timestamp_complete = my_timestamp_callback;
 * ret = exosite_timestamp(&exoLib);
 * \endcode
 */
int exosite_timestamp(Exosite_state_t *state);


#ifdef EXO_ASSERT_CHECKS
#define EXO_ASSERT(x) assert(x)
#else
#define EXO_ASSERT(...)
#endif
#endif /*__EXOSITE_ASYNC_H__*/
