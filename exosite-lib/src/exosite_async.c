/*****************************************************************************
*
*  exosite_async.c - Exosite library interface header
*  Copyright (C) 2017 Exosite LLC
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

#include <stdlib.h>
#include <limits.h>
#include <stdio.h> // FOR DEBUGGING ONLY.

#include "exosite_pal_async.h"
#include "exosite_async.h"


/******************************************************************************/

static const char STR_GET[] = "GET ";
static const char STR_POST[] = "POST ";
static const char STR_TIMESTAMP_URL[] = "/timestamp";
static const char STR_DATA_URL[] = "/onep:v1/stack/alias";
static const char STR_ACTIVATE_URL[] = "/provision/activate";
static const char STR_QM[] = "?";
static const char STR_HTTP[] = " HTTP/1.1";
static const char STR_HOST[] = "Host: ";
//static const char STR_HOST_ROOT[] = ".m2.exosite.com";
static const char STR_CIK_HEADER[] = "X-Exosite-CIK: ";
static const char STR_AGENT_HEADER[] = "User-Agent: Exosite-Async-C-lib/1.0";
static const char STR_REQUEST_TIMEOUT[] = "Request-Timeout: ";
static const char STR_MODIFIED_SINCE[] = "If-Modified-Since: ";
static const char STR_CONTENT_LENGTH[] = "Content-Length: ";
static const char STR_CONNECTION_CLOSE[] = "Connection: close";
static const char STR_ACCEPT[] = "Accept: application/x-www-form-urlencoded; charset=utf-8";
static const char STR_CONTENT[] = "Content-Type: application/x-www-form-urlencoded; charset=utf-8";
static const char STR_VENDOR[] = "vendor=";
static const char STR_MODEL[] = "&model=";
static const char STR_SN[] = "&sn=";
static const char STR_CRLF[] = "\r\n";

/******************************************************************************/
/* Local Prototypes */
void exosite_send_http_req(Exosite_state_t *state);
void exosite_http_rpl_init(exoHttp_rpl_t *http_rpl);
void exosite_http_rpl_header_name(Exosite_state_t *state, const char *data, size_t len);
void exosite_http_rpl_header_value(Exosite_state_t *state, const char *data, size_t len);
int exosite_http_rpl_body(Exosite_state_t *state, const char *data, size_t len);
void exosite_http_rpl_parse(Exosite_state_t *state, const char *data, size_t len);
void exosite_activate(Exosite_state_t *state);
uint8_t exosite_isCIKValid(char cik[CIK_LENGTH]);
int exosite_lib_start_complete(exoPal_state_t *pal, int status);
int exosite_lib_connected(exoPal_state_t *pal, int status);
int exosite_lib_send_complete(exoPal_state_t *pal, int status);
int exosite_lib_recv(exoPal_state_t *pal, const char *data, size_t len);
int exosite_lib_socket_closed(exoPal_state_t *pal, int status);
char exosite_to_lower(char c);

/******************************************************************************/
void exosite_send_http_req(Exosite_state_t *state)
{
    size_t slen = 0;
    exoHttp_req_t *req = &state->http.req;
    switch(req->step) {
        case exoHttp_req_method_url:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            if(req->is_post) {
                exoPal_strlcpy(state->workbuf, STR_POST, sizeof(state->workbuf));
            } else {
                exoPal_strlcpy(state->workbuf, STR_GET, sizeof(state->workbuf));
            }
            slen = exoPal_strlcat(state->workbuf, req->method_url_path, sizeof(state->workbuf));

            if(req->query != NULL) {
                req->step = exoHttp_req_query;
                slen = exoPal_strlcat(state->workbuf, STR_QM, sizeof(state->workbuf));
            } else {
                req->step = exoHttp_req_http_version;
            }
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_query:
            req->step = exoHttp_req_http_version;
            exoPal_socketWrite(state->exoPal, req->query, exoPal_strlen(req->query));
            break;

        case exoHttp_req_http_version:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            exoPal_strlcpy(state->workbuf, STR_HTTP, sizeof(state->workbuf));
            slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

            req->step = exoHttp_req_host;
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_host:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            slen = exoPal_strlcpy(state->workbuf, STR_HOST, sizeof(state->workbuf));
            exoPal_getHostname(&state->workbuf[slen], sizeof(state->workbuf)-slen);
            slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

            req->step = exoHttp_req_agent;
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_agent:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            exoPal_strlcpy(state->workbuf, STR_AGENT_HEADER, sizeof(state->workbuf));
            slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

            req->step = exoHttp_req_connection;
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_connection:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            exoPal_strlcpy(state->workbuf, STR_CONNECTION_CLOSE, sizeof(state->workbuf));
            slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

            req->step = exoHttp_req_cik;
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;


        case exoHttp_req_cik:
            req->step = exoHttp_req_content;
            // FIXME: If using TLS Client Certificate, then NEVER send this header.
            // Or if state->cik[0] == 0 ? // && strlen(state->cik) == 40 ???
            if(req->include_cik) {
                exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
                exoPal_strlcpy(state->workbuf, STR_CIK_HEADER, sizeof(state->workbuf));
                exoPal_strlcat(state->workbuf, state->cik, sizeof(state->workbuf));
                slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

                exoPal_socketWrite(state->exoPal, state->workbuf, slen);
                break;
            }
            // Fall-thru if no CIK

        case exoHttp_req_content:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            exoPal_strlcpy(state->workbuf, STR_CONTENT, sizeof(state->workbuf));
            slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

            req->step = exoHttp_req_accept;
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_accept:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            exoPal_strlcpy(state->workbuf, STR_ACCEPT, sizeof(state->workbuf));
            slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

            req->step = exoHttp_req_timeout;
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_timeout:
            req->step = exoHttp_req_modified_since;
            if(req->request_timeout > 0) {
                exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
                slen = exoPal_strlcpy(state->workbuf, STR_REQUEST_TIMEOUT, sizeof(state->workbuf));
                exoPal_itoa(req->request_timeout, &state->workbuf[slen], sizeof(state->workbuf)-slen);
                slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

                exoPal_socketWrite(state->exoPal, state->workbuf, slen);
                break;
            }
            // Fall-thru if no timeout

        case exoHttp_req_modified_since:
            req->step = exoHttp_req_content_length;
            if(req->modified_since != NULL) {
                exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
                exoPal_strlcpy(state->workbuf, STR_MODIFIED_SINCE, sizeof(state->workbuf));
                exoPal_strlcat(state->workbuf, req->modified_since, sizeof(state->workbuf));
                slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

                exoPal_socketWrite(state->exoPal, state->workbuf, slen);
                break;
            }
            // Fall-thru if no modified_since

        case exoHttp_req_content_length:
            req->step = exoHttp_req_start_body;
            if(req->content_length > 0) {
                exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
                slen = exoPal_strlcpy(state->workbuf, STR_CONTENT_LENGTH, sizeof(state->workbuf));
                exoPal_itoa(req->content_length, &state->workbuf[slen], sizeof(state->workbuf)-slen);
                slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

                exoPal_socketWrite(state->exoPal, state->workbuf, slen);
                break;
            }
            // Fall-thru if no content_length

        case exoHttp_req_start_body:
            req->step = exoHttp_req_body;
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            slen = exoPal_strlcpy(state->workbuf, STR_CRLF, sizeof(state->workbuf));
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_body:
            req->step = exoHttp_req_complete;
            if(req->is_activate) {
                exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
                exoPal_strlcpy(state->workbuf, STR_VENDOR, sizeof(state->workbuf));
                exoPal_strlcat(state->workbuf, state->projectid, sizeof(state->workbuf));
                exoPal_strlcat(state->workbuf, STR_MODEL, sizeof(state->workbuf));
                exoPal_strlcat(state->workbuf, state->modelid, sizeof(state->workbuf));
                exoPal_strlcat(state->workbuf, STR_SN, sizeof(state->workbuf));
                slen = exoPal_strlcat(state->workbuf, state->uuid, sizeof(state->workbuf));

                exoPal_socketWrite(state->exoPal, state->workbuf, slen);
                break;
            } else if(req->body != NULL) {
                exoPal_socketWrite(state->exoPal, req->body, exoPal_strlen(req->body));
                break;
            }
            // Fall-thru if no body

        case exoHttp_req_complete:
            exoPal_sendingComplete(state->exoPal);
            state->stage = Exosite_Stage_waiting;
            break;
    }
}
/******************************************************************************/

void exosite_http_rpl_init(exoHttp_rpl_t *http_rpl)
{
    exoPal_memset(http_rpl, 0, sizeof(exoHttp_rpl_t));
}

void exosite_http_rpl_header_name(Exosite_state_t *state, const char *data, size_t len)
{
//    printf("THUNK -> Header Name: %.*s\n", (int)len, data);
}
void exosite_http_rpl_header_value(Exosite_state_t *state, const char *data, size_t len)
{
//    printf("THUNK -> Header value: %.*s\n", (int)len, data);
}

char exosite_to_lower(char c) {
    if(c >= 'A' && c <= 'Z') {
        return c + ('a'-'A');
    }
    return c;
}

void exosite_http_rpl_parse(Exosite_state_t *state, const char *data, size_t len)
{
    exoHttp_rpl_t *rpl = &state->http.rpl;
    char dc;
    const char *start_mark = data;

    // Special case speed up. If in the exoHttp_rpl_body, nothing to scan.
    if(rpl->step == exoHttp_rpl_body) {
        rpl->counter_content_length -= len;
        if(exosite_http_rpl_body(state, data, len) != 0) {
            rpl->step = exoHttp_rpl_complete;
        }
        if(rpl->counter_content_length <= 0) {
            rpl->step = exoHttp_rpl_complete;
        }
        return;
    }

    for(; len > 0; ++data, --len) {
        dc = *data;
        switch(rpl->step) {
            case exoHttp_rpl_looking_for_status:
                if(dc == ' ') {
                    rpl->step = exoHttp_rpl_read_status;
                }
                break;
            case exoHttp_rpl_read_status:
                if(dc >= '0' && dc <= '9') {
                    rpl->statusCode *= 10;
                    rpl->statusCode += dc - '0';
                } else if(dc == ' ') {
                    rpl->step = exoHttp_rpl_status_looking_for_cr;
                } else {
                    rpl->step = exoHttp_rpl_error;
                }
                break;
            case exoHttp_rpl_status_looking_for_cr:
                if(dc == '\r') {
                    rpl->step = exoHttp_rpl_status_looking_for_lf;
                }
                break;
            case exoHttp_rpl_status_looking_for_lf:
                if(dc != '\n') {
                    rpl->step = exoHttp_rpl_error;
                } else {
                    rpl->step = exoHttp_rpl_header_start;
                }
                break;
            case exoHttp_rpl_header_start:
                if(dc == '\r') {
                    // This is either start of second CRLF to start body, or will be
                    // error.
                    rpl->step = exoHttp_rpl_looking_for_lf_start_body;
                } else {
                    // Watch for Content-Length.
                    if(exosite_to_lower(dc) == 'c') {
                        rpl->counter_content_length = 1;
                    } else {
                        rpl->counter_content_length = -1;
                    }
                    start_mark = data;
                    rpl->step = exoHttp_rpl_header_looking_for_sep;
                }
                break;
            case exoHttp_rpl_header_looking_for_sep:
                if(dc == ':') {
                    // switch to value
                    rpl->step = exoHttp_rpl_header_mark_value;
                    exosite_http_rpl_header_name(state, start_mark, data-start_mark);
                } else if(rpl->counter_content_length >= 0) {
                    // Still Content-Length?
                    if( exosite_to_lower(dc) == exosite_to_lower(STR_CONTENT_LENGTH[rpl->counter_content_length])) {
                        rpl->counter_content_length ++;
                    } else {
                        rpl->counter_content_length = -1;
                    }
                }
                break;
            case exoHttp_rpl_header_mark_value:
                start_mark = data;
                rpl->step = exoHttp_rpl_header_looking_for_cr;
                if(rpl->counter_content_length > 0) {
                    rpl->content_length = 0;
                }
                // this *must* fall-thru
            case exoHttp_rpl_header_looking_for_cr:
                if(dc == '\r') {
                    rpl->step = exoHttp_rpl_header_looking_for_lf;
                    exosite_http_rpl_header_value(state, start_mark, data-start_mark);
                } else if(rpl->counter_content_length > 0) {
                    // This is Content-Length
                    if(dc >= '0' && dc <= '9') {
                        rpl->content_length *= 10;
                        rpl->content_length += dc - '0';
                    }
                }
                break;
            case exoHttp_rpl_header_looking_for_lf:
                if(dc == '\n') {
                    // end of header.
                    // Next header, or maybe body.
                    rpl->step = exoHttp_rpl_header_start;
                } else {
                    rpl->step = exoHttp_rpl_error;
                }
                rpl->counter_content_length = -1;
                break;


            case exoHttp_rpl_looking_for_lf_start_body:
                if(dc != '\n') {
                    rpl->step = exoHttp_rpl_error;
                } else {
                    rpl->step = exoHttp_rpl_mark_body;
                }
                break;
            case exoHttp_rpl_mark_body:
                rpl->step = exoHttp_rpl_body;
                start_mark = data;
                if(rpl->content_length > 0) {
                    rpl->counter_content_length = rpl->content_length;
                } else {
                    rpl->counter_content_length = INT_MAX;
                }
                // this *must* fall-thru
            case exoHttp_rpl_body:
                // just get to the end now.
                break;

            case exoHttp_rpl_complete:
            case exoHttp_rpl_error:
                break;
        }
    }
    // Handle partial callbacks.
    switch(rpl->step) {
        case exoHttp_rpl_header_looking_for_sep:
            exosite_http_rpl_header_name(state, start_mark, data-start_mark);
            break;

        case exoHttp_rpl_header_looking_for_cr:
            exosite_http_rpl_header_value(state, start_mark, data-start_mark);
            break;

        case exoHttp_rpl_body:
            rpl->counter_content_length -= data-start_mark;
            if(exosite_http_rpl_body(state, start_mark, data-start_mark) != 0) {
                rpl->step = exoHttp_rpl_complete;
            }
            if(rpl->counter_content_length <= 0) {
                rpl->step = exoHttp_rpl_complete;
            }
            break;

        default:
            // nop.
            break;
    }
}

/******************************************************************************/

void exosite_activate(Exosite_state_t *state)
{
    state->state = Exosite_State_activate;
    state->stage = Exosite_Stage_connecting;

    state->http.req.step = exoHttp_req_method_url;
    state->http.req.method_url_path = (char*)STR_ACTIVATE_URL;
    state->http.req.content_length = (sizeof(STR_VENDOR) - 1) +
        (sizeof(STR_MODEL) - 1) + (sizeof(STR_SN) - 1) +
        exoPal_strlen(state->projectid) +
        exoPal_strlen(state->modelid) +
        exoPal_strlen(state->uuid);
    state->http.req.is_post = 1;
    state->http.req.include_cik = 0;
    state->http.req.is_activate = 1;
    state->http.req.body = NULL;
    state->http.req.query = NULL;
    state->http.req.modified_since = NULL;
    state->http.req.request_timeout = 0;

    exoPal_tcpSocketOpen(state->exoPal);
}

/******************************************************************************/

/*!
 * \brief Checks if the given cik is valid
 *
 * Checks that the first 40 chars of `cik` are valid, lowercase hexadecimal bytes.
 *
 *
 *
 * \param[in] cik array of CIK_LENGTH bytes
 *
 * \return Returns 1 if successful, else 0
 * \sa
 * \note
 * \warning
 */
uint8_t exosite_isCIKValid(char cik[CIK_LENGTH])
{
    uint8_t i;

    for (i = 0; i < CIK_LENGTH; i++)
    {
        if (!((cik[i] >= 'a' && cik[i] <= 'f') || (cik[i] >= '0' && cik[i] <= '9')))
        {
            return 0;
        }
    }

    return 1;
}

/******************************************************************************/
int exosite_lib_start_complete(exoPal_state_t *pal, int status)
{
    Exosite_state_t *state = (Exosite_state_t *)pal->context;
    EXO_ASSERT(pal->context);

    if(status != 0) {
        if(state->ops.on_start_complete) {
            state->ops.on_start_complete(state, status);
        }
        return -1;
    }

    // Everything should be ready for network IO now.
    // We always do an activate next.
    exosite_activate(state);
    return 0;
}

int exosite_lib_connected(exoPal_state_t *pal, int status)
{
    Exosite_state_t *state = (Exosite_state_t *)pal->context;
    EXO_ASSERT(pal->context);

    state->stage = Exosite_Stage_sending;
    exosite_send_http_req(state);
    return 0;
}

int exosite_lib_send_complete(exoPal_state_t *pal, int status)
{
    Exosite_state_t *state = (Exosite_state_t *)pal->context;
    EXO_ASSERT(pal->context);

    exosite_send_http_req(state);

    if(state->stage == Exosite_Stage_waiting) {
        state->stage = Exosite_Stage_recving;
        state->wb_offset = 0;
        state->statusCode = 0;
        exosite_http_rpl_init(&state->http.rpl);
        exoPal_socketRead(state->exoPal, state->workbuf, sizeof(state->workbuf));
    }
    return 0;
}

int exosite_lib_recv(exoPal_state_t *pal, const char *data, size_t len)
{
    Exosite_state_t *state = (Exosite_state_t *)pal->context;
    EXO_ASSERT(pal->context);

    // len == 0 means EOS, so close socket.
    if(len == 0) {
        state->stage = Exosite_Stage_closing;
        exoPal_tcpSocketClose(state->exoPal);
        return 0;
    }

    exosite_http_rpl_parse(state, data, len);

    if(state->http.rpl.step == exoHttp_rpl_complete) {
        state->stage = Exosite_Stage_closing;
        exoPal_tcpSocketClose(state->exoPal);

    } else if(state->http.rpl.step == exoHttp_rpl_error) {
        state->stage = Exosite_Stage_closing;
        exoPal_tcpSocketClose(state->exoPal);
        // TODO: callback?

    } else {
        exoPal_socketRead(state->exoPal, state->workbuf, sizeof(state->workbuf));
    }
    return 0;
}


int exosite_http_rpl_body(Exosite_state_t *state, const char *data, size_t len)
{
    int retcode = 0;
    if(state->stage == Exosite_Stage_recving) {
        state->stage = Exosite_Stage_recving_body;

        // Fire off read_begin callbacks.
        switch(state->state) {
            case Exosite_State_read:
            case Exosite_State_hybrid:
            case Exosite_State_longpoll:
                if(state->ops.on_read_begin) {
                    state->ops.on_read_begin(state, state->http.rpl.statusCode);
                }
                break;
            default:
                // nop
                break;
        }
    }

    // Process body data.
    switch(state->state) {
        case Exosite_State_activate:
            if(state->http.rpl.statusCode == 409) {
                // Already activated.
                exoPal_getCik(state->cik, CIK_LENGTH);
                if(!exosite_isCIKValid(state->cik)) {
                    // Stored CIK is invalid and this SN is activated.
                    state->statusCode = -409;
                }

                // activate complete!
                state->statusCode = 200;
                retcode = 1;

            } else if(state->http.rpl.statusCode == 200) {
                // Just activated.

                // copy into cik, by parts if need be.
                unsigned int remain = MIN(len, (unsigned int)(CIK_LENGTH - state->wb_offset));
                EXO_ASSERT(remain > 0);
                exoPal_memmove(&state->cik[state->wb_offset], data, remain);
                state->wb_offset += remain;
                EXO_ASSERT(state->wb_offset <= CIK_LENGTH);

                if(state->wb_offset == CIK_LENGTH) {
                    // ok, all of CIK is now in cik; save it.
                    if(exosite_isCIKValid(state->cik)) {
                        exoPal_setCik(state->cik, CIK_LENGTH);
                        state->statusCode = 200;
                    } else {
                        // It gave us an invalid CIK!?!?!?!
                        state->statusCode = -200;
                    }

                    retcode = 1;
                }
            } else if(state->http.rpl.statusCode == 415) {
                // We identified with a TLS Client Certificate.
                // Carry on.
                state->statusCode = 200;
                retcode = 1;

            } else {
                // All other status codes.
                state->statusCode = state->http.rpl.statusCode;
                retcode = 1;
            }
            break;

        case Exosite_State_write:
            // If they send back a body, we ignore it.
            retcode = 1;
            break;

        case Exosite_State_read:
        case Exosite_State_hybrid:
        case Exosite_State_longpoll:
            if(state->http.rpl.statusCode != 200) {
                // just stop.
                retcode = 1;

            } else {
                if(state->ops.on_read_raw) {
                    state->ops.on_read_raw(state, data, len);
                }
                // TODO: decode form data.
                // Do this in here? or provide a layer above?
            }
            break;

        case Exosite_State_timestamp:
            if(state->http.rpl.statusCode != 200) {
                // just stop.
                retcode = 1;

            } else {
                // The actual timestamp values could be broken across multiple
                // calls of this function.

                for(; len > 0; ++data, --len) {
                    char dc = *data;
                    if(dc >= '0' && dc <= '9') {
                        state->timestamp *= 10;
                        state->timestamp += dc - '0';
                    } else {
                        // Not number, stop.
                        retcode = 1;
                        break;
                    }
                }
            }
            break;

        default:
            EXO_ASSERT(0);
            break;
    }
    return retcode;
}

int exosite_lib_socket_closed(exoPal_state_t *pal, int status)
{
    Exosite_state_t *state = (Exosite_state_t *)pal->context;
    EXO_ASSERT(pal->context);

    switch(state->state) {
        case Exosite_State_activate:
            state->stage = Exosite_Stage_idle;
            state->state = Exosite_State_idle;
            if(state->ops.on_start_complete) {
                state->ops.on_start_complete(state, state->statusCode);
            }
            break;

        case Exosite_State_write:
            state->stage = Exosite_Stage_idle;
            state->state = Exosite_State_idle;
            if(state->ops.on_write_complete) {
                state->ops.on_write_complete(state, state->http.rpl.statusCode);
            }
            break;

        case Exosite_State_timestamp:
            state->stage = Exosite_Stage_idle;
            state->state = Exosite_State_idle;
            if(state->ops.on_timestamp_complete) {
                state->ops.on_timestamp_complete(state, state->timestamp);
            }
            break;

        case Exosite_State_read:
        case Exosite_State_hybrid:
        case Exosite_State_longpoll:
            state->stage = Exosite_Stage_idle;
            state->state = Exosite_State_idle;
            if(state->ops.on_read_complete) {
                state->ops.on_read_complete(state, state->statusCode);
            }
            break;

        default:
            EXO_ASSERT(0);
            break;
    }
    return 0;
}

/******************************************************************************/

void exosite_init(Exosite_state_t *state)
{
    exoPal_memset(state, 0, sizeof(Exosite_state_t));
}

int exosite_start(Exosite_state_t *state)
{
    if(state->state != Exosite_State_initialized) {
        return -1;
    }

    // Setup pal callbacks.
    if(state->exoPal == NULL) {
        return -1;
    }
    exoPal_init(state->exoPal);

    state->exoPal->ops.on_start_complete = exosite_lib_start_complete;
    state->exoPal->ops.on_connected = exosite_lib_connected;
    state->exoPal->ops.on_send_complete = exosite_lib_send_complete;
    state->exoPal->ops.on_recv = exosite_lib_recv;
    state->exoPal->ops.on_socket_closed = exosite_lib_socket_closed;
    state->exoPal->context = state;

    // Build full domain name string
    exoPal_getVendor(state->projectid, MAX_VENDOR_LENGTH);
    state->projectid[MAX_VENDOR_LENGTH-1] = '\0';
    exoPal_getModel(state->modelid, MAX_MODEL_LENGTH);
    state->modelid[MAX_MODEL_LENGTH-1] = '\0';

    // Need the SN.
    exoPal_getUuid(state->uuid, MAX_UUID_LENGTH);

    state->state = Exosite_State_pal_starting;
    // Start up pal. (includes a DNS lookup.)
    return exoPal_start(state->exoPal);
}

int exosite_write(Exosite_state_t *state, const char *aliasesAndValues)
{
    if(state->state != Exosite_State_idle) {
        return -1;
    }

    state->state = Exosite_State_write;
    state->stage = Exosite_Stage_connecting;

    state->http.req.step = exoHttp_req_method_url;
    state->http.req.method_url_path = (char*)STR_DATA_URL;
    state->http.req.content_length = exoPal_strlen(aliasesAndValues);
    state->http.req.is_post = 1;
    state->http.req.include_cik = 1;
    state->http.req.is_activate = 0;
    state->http.req.body = aliasesAndValues;
    state->http.req.query = NULL;
    state->http.req.modified_since = NULL;
    state->http.req.request_timeout = 0;

    state->statusCode = 0;

    return exoPal_tcpSocketOpen(state->exoPal);
}

int exosite_read(Exosite_state_t *state, const char *aliases)
{
    if(state->state != Exosite_State_idle) {
        return -1;
    }

    state->state = Exosite_State_read;
    state->stage = Exosite_Stage_connecting;

    state->http.req.step = exoHttp_req_method_url;
    state->http.req.method_url_path = (char*)STR_DATA_URL;
    state->http.req.content_length = 0;
    state->http.req.is_post = 0;
    state->http.req.include_cik = 1;
    state->http.req.is_activate = 0;
    state->http.req.body = NULL;
    state->http.req.query = aliases;
    state->http.req.modified_since = NULL;
    state->http.req.request_timeout = 0;

    state->statusCode = 0;

    return exoPal_tcpSocketOpen(state->exoPal);
}

int exosite_hybrid(Exosite_state_t *state, const char *writeAliasesAndValues, const char *readAliases)
{
    if(state->state != Exosite_State_idle) {
        return -1;
    }

    state->state = Exosite_State_hybrid;
    state->stage = Exosite_Stage_connecting;

    state->http.req.step = exoHttp_req_method_url;
    state->http.req.method_url_path = (char*)STR_DATA_URL;
    state->http.req.content_length = exoPal_strlen(writeAliasesAndValues);
    state->http.req.is_post = 1;
    state->http.req.include_cik = 1;
    state->http.req.is_activate = 0;
    state->http.req.body = writeAliasesAndValues;
    state->http.req.query = readAliases;
    state->http.req.modified_since = NULL;
    state->http.req.request_timeout = 0;

    state->statusCode = 0;

    return exoPal_tcpSocketOpen(state->exoPal);
}

int exosite_longpoll(Exosite_state_t *state, const char *alias, const char *modified_since, uint32_t timeout)
{
    if(state->state != Exosite_State_idle) {
        return -1;
    }

    state->state = Exosite_State_longpoll;
    state->stage = Exosite_Stage_connecting;

    state->http.req.step = exoHttp_req_method_url;
    state->http.req.method_url_path = (char*)STR_DATA_URL;
    state->http.req.content_length = 0;
    state->http.req.is_post = 0;
    state->http.req.include_cik = 1;
    state->http.req.is_activate = 0;
    state->http.req.body = NULL;
    state->http.req.query = alias;
    state->http.req.modified_since = modified_since;
    if(timeout == 0) {
        timeout = 300000;
    }
    state->http.req.request_timeout = MIN(300000, timeout);

    state->statusCode = 0;

    return exoPal_tcpSocketOpen(state->exoPal);
}

int exosite_timestamp(Exosite_state_t *state)
{
    if(state->state != Exosite_State_idle) {
        return -1;
    }

    state->state = Exosite_State_timestamp;
    state->stage = Exosite_Stage_connecting;

    state->http.req.step = exoHttp_req_method_url;
    state->http.req.method_url_path = (char*)STR_TIMESTAMP_URL;
    state->http.req.content_length = 0;
    state->http.req.is_post = 0;
    state->http.req.include_cik = 0;
    state->http.req.is_activate = 0;
    state->http.req.body = NULL;
    state->http.req.query = NULL;
    state->http.req.modified_since = NULL;
    state->http.req.request_timeout = 0;

    state->statusCode = 0;
    state->timestamp = 0;

    return exoPal_tcpSocketOpen(state->exoPal);
}

/* vim: set ai cin et sw=4 ts=4 : */
