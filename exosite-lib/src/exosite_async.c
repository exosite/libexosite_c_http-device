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

#include "exosite_pal.h"
#include "exosite_async.h"



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


enum exoHttp_req_e {
    exoHttp_req_method_url = 0,
    exoHttp_req_host,
    exoHttp_req_cik,
    exoHttp_req_content,
    exoHttp_req_content_length,
    exoHttp_req_body,
    exoHttp_req_complete
};
struct http_req_s {
    enum exoHttp_req_e step;
    char * method_url_path;
    int include_cik;
    size_t content_length;
    char *body;
};

/******************************************************************************/
void exosite_send_http_req(Exosite_state_t *state)
{
    size_t slen = 0;
    struct http_req_s *req = (struct http_req_s*)state->http_req;
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
            exoPal_strlcpy(state->workbuf, STR_HOST_ROOT, sizeof(state->workbuf));
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
            exoPal_socketWrite(state->exoPal, req->body, exoPal_strlen(req->body));
            break;

        case exoHttp_req_complete:
            exoPal_sendingComplete(state->exoPal);
            state->stage = Exosite_Stage_waiting;
            break;
    }
}

void exosite_activate(Exosite_state_t *state)
{
    // - Connect socket
    state->state = Exosite_State_activate;
    state->stage = Exosite_Stage_connecting;
    exoPal_tcpSocketOpen(state->exoPal);

    // - Write data
    // - wait for reply
    // - parse it.
    // - call cb with results.
}
void exosite_activate_send(Exosite_state_t *state)
{
    struct http_req_s *req;
    req->step = exoHttp_req_method_url;
    req->method_url_path = (char*)STR_ACTIVATE_URL;
    req->include_cik = 0;
    req->content_length = 35; // XXX
    req->body = (char*)STR_VENDOR; // omg. this sux.

    exosite_send_http_req(state);
}

/******************************************************************************/
int exosite_lib_start_complete(exoPal_state_t *pal, int status)
{
    Exosite_state_t *state = (Exosite_state_t *)pal->context;
    EXO_ASSERT(pal->context);

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
                exoPal_socketRead(state->exoPal, state->workbuf, sizeof(state->workbuf));
            }

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

        case Exosite_State_write:
            // Find status code. Toss rest.

        default:
            EXO_ASSERT(0);
            break;
    }
    return 0;
}

int exosite_lib_socket_closed(exoPal_state_t *pal, int status)
{
    return 0;
}

/******************************************************************************/
#if 0
void exosite_init(Exosite_state_t *state)
{
    exoPal_memset(state, 0, sizeof(Exosite_state_t));
}
#endif

int exosite_start(Exosite_state_t *state)
{
    char hostbuf[MAX_VENDOR_LENGTH + 15 + 1];
    state->state = Exosite_State_needs_start;

    // Build full domain name string
    exoPal_getProduct(state->projectid);
    state->projectid[MAX_VENDOR_LENGTH-1] = '\0';
    exoPal_strlcpy(hostbuf, state->projectid, sizeof(hostbuf));
    exoPal_strlcat(hostbuf, STR_HOST_ROOT, sizeof(STR_HOST_ROOT));

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


    state->state = Exosite_State_dns_lookup;
    // Start up pal. (includes a DNS lookup.)
    exoPal_start(state->exoPal, hostbuf);

    return 0;
}
/* vim: set ai cin et sw=4 ts=4 : */
