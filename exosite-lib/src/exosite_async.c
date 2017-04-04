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

#include "exosite_pal_async.h"
#include "exosite_async.h"


/******************************************************************************/

static const char STR_TIMESTAMP_URL[] = "GET /timestamp ";
static const char STR_CIK_HEADER[] = "X-Exosite-CIK: ";
static const char STR_CONTENT_LENGTH[] = "Content-Length: ";
static const char STR_READ_URL[] = "GET /onep:v1/stack/alias?";
static const char STR_WRITE_URL[] = "POST /onep:v1/stack/alias ";
static const char STR_ACTIVATE_URL[] = "POST /provision/activate ";
static const char STR_HTTP[] = "HTTP/1.1";
static const char STR_HOST[] = "Host: ";
static const char STR_HOST_ROOT[] = ".m2.exosite.com";
static const char STR_ACCEPT[] = "Accept: application/x-www-form-urlencoded; charset=utf-8";
static const char STR_CONTENT[] = "Content-Type: application/x-www-form-urlencoded; charset=utf-8";
static const char STR_VENDOR[] = "vendor=";
static const char STR_MODEL[] = "&model=";
static const char STR_SN[] = "&sn=";
static const char STR_CRLF[] = "\r\n";


/******************************************************************************/
void exosite_send_http_req(Exosite_state_t *state)
{
    size_t slen = 0;
    exoHttp_req_t *req = &state->http_req;
    switch(req->step) {
        case exoHttp_req_method_url:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            exoPal_strlcpy(state->workbuf, req->method_url_path, sizeof(state->workbuf));
            exoPal_strlcat(state->workbuf, STR_HTTP, sizeof(state->workbuf));
            slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

            req->step = exoHttp_req_host;
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_host:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            exoPal_strlcpy(state->workbuf, STR_HOST, sizeof(state->workbuf));
            exoPal_strlcat(state->workbuf, state->projectid, sizeof(state->workbuf));
            exoPal_strlcat(state->workbuf, STR_HOST_ROOT, sizeof(state->workbuf));
            slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

            if(req->include_cik) {
                req->step = exoHttp_req_cik;
            } else {
                req->step = exoHttp_req_content;
            }
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_cik:
            req->step = exoHttp_req_content;
            exoPal_socketWrite(state->exoPal, STR_CONTENT, sizeof(STR_CONTENT)-1);
            break;

        case exoHttp_req_content:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            exoPal_strlcpy(state->workbuf, STR_CONTENT, sizeof(state->workbuf));
            slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

            req->step = exoHttp_req_content_length;
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_content_length:
            exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
            slen = exoPal_strlcpy(state->workbuf, STR_CONTENT_LENGTH, sizeof(state->workbuf));
            exoPal_itoa(req->content_length, &state->workbuf[slen], sizeof(state->workbuf)-slen);
            // Two CRLF because body is next
            exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));
            slen = exoPal_strlcat(state->workbuf, STR_CRLF, sizeof(state->workbuf));

            req->step = exoHttp_req_body;
            exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            break;

        case exoHttp_req_body:
            req->step = exoHttp_req_complete;
            if(req->is_activate) {
                exoPal_memset(state->workbuf, 0, sizeof(state->workbuf));
                exoPal_strlcpy(state->workbuf, STR_VENDOR, sizeof(state->workbuf));
                exoPal_strlcat(state->workbuf, state->projectid, sizeof(state->workbuf));
                exoPal_strlcpy(state->workbuf, STR_MODEL, sizeof(state->workbuf));
                exoPal_strlcat(state->workbuf, state->projectid, sizeof(state->workbuf));
                exoPal_strlcpy(state->workbuf, STR_SN, sizeof(state->workbuf));
                slen = exoPal_strlcat(state->workbuf, state->uuid, sizeof(state->workbuf));

                exoPal_socketWrite(state->exoPal, state->workbuf, slen);
            } else {
                exoPal_socketWrite(state->exoPal, req->body, exoPal_strlen(req->body));
            }
            break;

        case exoHttp_req_complete:
            exoPal_sendingComplete(state->exoPal);
            state->stage = Exosite_Stage_waiting;
            break;
    }
}

void exosite_activate(Exosite_state_t *state)
{
    state->state = Exosite_State_activate;
    state->stage = Exosite_Stage_connecting;
    exoPal_tcpSocketOpen(state->exoPal);
}
void exosite_activate_send(Exosite_state_t *state)
{
    state->http_req.step = exoHttp_req_method_url;
    state->http_req.method_url_path = (char*)STR_ACTIVATE_URL;
    state->http_req.content_length = (sizeof(STR_VENDOR) - 1) +
        (sizeof(STR_MODEL) - 1) + (sizeof(STR_SN) - 1) +
        (exoPal_strlen(state->projectid) * 2) + exoPal_strlen(state->uuid);
    state->http_req.include_cik = 0;
    state->http_req.is_activate = 1;
    state->http_req.body = NULL;

    exosite_send_http_req(state);
}

/******************************************************************************/

/*!
 * \brief Gets the HTTP status code out of the buffer
 *
 * \param[in] response an HTTP response string
 *
 * This assumes that response starts from the beginning of the response data and
 * that the status code is within the first 15 bytes.
 *
 * \return [uint16_t] The status code.
 *
 */
uint16_t exosite_getStatusCode(char * response)
{
    uint8_t spaceFound = 0;
    int16_t i;
    for (i = 1; (i < 15) && (spaceFound == 0); i++)
    {
        if (response[i] == ' ')
        {
            spaceFound = i + 1;
        }
    }
    if (spaceFound > 0)
    {
        // If we found a ' ', try to match the code
        return exoPal_atoi(&response[spaceFound]);
    }

    return 0;
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

    switch(state->state) {
        case Exosite_State_activate:
            state->stage = Exosite_Stage_sending;
            exosite_activate_send(state);
            break;

        case Exosite_State_write:
            state->stage = Exosite_Stage_sending;
            exosite_send_http_req(state);
            break;

        case Exosite_State_timestamp:
            state->stage = Exosite_Stage_sending;
            exosite_send_http_req(state);
            break;

        default:
            EXO_ASSERT(0);
            break;
    }
    return 0;
}

int exosite_lib_send_complete(exoPal_state_t *pal, int status)
{
    Exosite_state_t *state = (Exosite_state_t *)pal->context;
    EXO_ASSERT(pal->context);

    switch(state->state) {
        case Exosite_State_activate:
            exosite_send_http_req(state);

            if(state->stage == Exosite_Stage_waiting) {
                state->stage = Exosite_Stage_recving_headers;
                state->wb_offset = 0;
                exoPal_socketRead(state->exoPal, state->workbuf, sizeof(state->workbuf));
            }
            break;

        case Exosite_State_write:
            exosite_send_http_req(state);

            if(state->stage == Exosite_Stage_waiting) {
                state->stage = Exosite_Stage_recving_headers;
                state->wb_offset = 0;
                exoPal_socketRead(state->exoPal, state->workbuf, sizeof(state->workbuf));
            }
            break;

        case Exosite_State_timestamp:
            exosite_send_http_req(state);

            if(state->stage == Exosite_Stage_waiting) {
                state->stage = Exosite_Stage_recving_headers;
                state->wb_offset = 0;
                exoPal_socketRead(state->exoPal, state->workbuf, sizeof(state->workbuf));
            }
            break;

        default:
            EXO_ASSERT(0);
            break;
    }
    return 0;
}

int exosite_lib_recv(exoPal_state_t *pal, const char *data, size_t len)
{
    Exosite_state_t *state = (Exosite_state_t *)pal->context;
    EXO_ASSERT(pal->context);

    switch(state->state) {
        case Exosite_State_activate:
            // find status code. Maybe find body.
            // "HTTP/* ### *\r\n
            if (state->stage == Exosite_Stage_recving_headers) {
                if( (state->wb_offset + len) < 15 ) {
                    state->wb_offset += len;
                    exoPal_socketRead(state->exoPal,
                            &(state->workbuf[state->wb_offset]),
                            sizeof(state->workbuf) - state->wb_offset);
                } else {
                    // Ok, we have the HTTP status code
                    int statusCode = exosite_getStatusCode(state->workbuf);
                    if(statusCode == 409) {
                        // Already activated.
                        exoPal_getCik(state->cik);
                        // TODO: isvalid?

                        // activate complete!
                        state->stage = Exosite_Stage_closing;
                        exoPal_tcpSocketClose(state->exoPal);

                    } else if(statusCode == 200) {
                        // Just activated.  Now need to find body and get CIK.
                        state->stage = Exosite_Stage_recving_body;

                        // TODO: now what?
                        state->stage = Exosite_Stage_closing;
                        exoPal_tcpSocketClose(state->exoPal);
                    }
                }
            }
            break;

        case Exosite_State_write:
            // Find status code. Toss rest.
            if (state->stage == Exosite_Stage_recving_headers) {
                if( (state->wb_offset + len) < 15 ) {
                    state->wb_offset += len;
                    exoPal_socketRead(state->exoPal,
                            &(state->workbuf[state->wb_offset]),
                            sizeof(state->workbuf) - state->wb_offset);
                } else {
                    // Ok, we have the HTTP status code
                    // We'll close things up here, and pull out the code later.
                    state->stage = Exosite_Stage_closing;
                    exoPal_tcpSocketClose(state->exoPal);
                }
            }
            break;

        case Exosite_State_timestamp:
            // Find status code. Then find body
            if (state->stage == Exosite_Stage_recving_headers) {
                if( (state->wb_offset + len) < 15 ) {
                    state->wb_offset += len;
                    exoPal_socketRead(state->exoPal,
                            &(state->workbuf[state->wb_offset]),
                            sizeof(state->workbuf) - state->wb_offset);
                } else {
                    // Ok, we have the HTTP status code
                    int statusCode = exosite_getStatusCode(state->workbuf);
                    if(statusCode == 200) {
                        state->stage = Exosite_Stage_recving_body;
                        // TODO: Get the body.
                    } else {
                        // error.
                    }
                }
            }
            break;

        default:
            EXO_ASSERT(0);
            break;
    }
    return 0;
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
                state->ops.on_start_complete(state, 0);
            }
            break;

        case Exosite_State_write:
            state->stage = Exosite_Stage_idle;
            state->state = Exosite_State_idle;
            if(state->ops.on_write_complete) {
                int statusCode = exosite_getStatusCode(state->workbuf);
                state->ops.on_write_complete(state, statusCode);
            }

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
    char hostbuf[MAX_VENDOR_LENGTH + sizeof(STR_HOST_ROOT) + 1];

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
    exoPal_getProduct(state->projectid);
    state->projectid[MAX_VENDOR_LENGTH-1] = '\0';
    exoPal_strlcpy(hostbuf, state->projectid, sizeof(hostbuf));
    exoPal_strlcat(hostbuf, STR_HOST_ROOT, sizeof(STR_HOST_ROOT));

    state->state = Exosite_State_pal_starting;
    // Start up pal. (includes a DNS lookup.)
    exoPal_start(state->exoPal, hostbuf);

    return 0;
}

void exosite_write(Exosite_state_t *state, const char *aliasesAndValues)
{
    state->state = Exosite_State_write;
    state->stage = Exosite_Stage_connecting;

    state->http_req.step = exoHttp_req_method_url;
    state->http_req.method_url_path = (char*)STR_WRITE_URL;
    state->http_req.content_length = exoPal_strlen(aliasesAndValues);
    state->http_req.include_cik = 1;
    state->http_req.is_activate = 0;
    state->http_req.body = aliasesAndValues;

    exoPal_tcpSocketOpen(state->exoPal);
}

void exosite_timestamp(Exosite_state_t *state)
{
    state->state = Exosite_State_timestamp;
    state->stage = Exosite_Stage_connecting;

    state->http_req.step = exoHttp_req_method_url;
    state->http_req.method_url_path = (char*)STR_TIMESTAMP_URL;
    state->http_req.content_length = 0;
    state->http_req.include_cik = 0;
    state->http_req.is_activate = 0;
    state->http_req.body = NULL;

    exoPal_tcpSocketOpen(state->exoPal);
}

/* vim: set ai cin et sw=4 ts=4 : */
