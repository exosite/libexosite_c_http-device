/*****************************************************************************
*
*  exosite_pal_async.c - Common header for Exosite hardware adapation layer
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
#include "exosite_pal_async.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

/*****************************************************************************
 * \defgroup Utility PAL functions
 * @{
 */

/** \brief Get string form of an integer
 * \param[in] value Number that you want a string representation of
 * \param[out] buf Buffer to place results
 * \param[in] bufSizelength of buffer
 * \return Length of string written to buf
 */
uint8_t exoPal_itoa(int value, char* buf, uint8_t bufSize)
{
    int32_t chars = snprintf(buf, bufSize, "%d", value);
    if (chars < 0)
    {
        return 0;
    }
    return chars;
}

/** \brief Get interger from a string.
 * \param[in] val String to get integer from.
 * \return integer or 0
 */
int32_t exoPal_atoi(char* val)
{
    return atoi(val);
}

/** \brief Compute length of string
 * \param[in] s String to compute length of
 * \return Length of string not including NUL
 */
size_t exoPal_strlen(const char *s)
{
    return strlen(s);
}

/** \brief Find string within string
 * \param[in] str String to search within
 * \param[in] target The string to find
 * \return NULL if not found, otherwise pointer to where found
 */
char* exoPal_strstr(const char *str, const char *target)
{
    return strstr(str, target);
}

/** \brief Copy memory from possibly overlapping regions
 * \param[out] dst Where to copy to
 * \param[in] src Where to copy from
 * \param[in] length How much to copy
 * \return dst
 */
void * exoPal_memmove(void* dst, const void * src, size_t length)
{
    return memmove(dst, src, length);
}

/** \brief Write bytes of value
 * \param[out] dst Where to write
 * \param[in] c What byte value to write
 * \param[in] length How many bytes to write with #c
 * \return dst
 */
void * exoPal_memset(void* dst, int c, uint16_t length)
{
    return memset(dst, c, length);
}

/** \brief Copy string
 * \param[out] dst Where to copy to
 * \param[in] src What to copy
 * \param[in] len Maximum size of the buffer #dst
 *
 * This copies at most #len - 1 bytes from #src.
 *
 * \return length of copied string.
 */
size_t exoPal_strlcpy(char* dst, const char* src, size_t len)
{
    return strlcpy(dst, src, len);
}

/** \brief Concatenate strings
 * \param[in,out] dst Where to concatenate onto
 * \param[in] src What to copy
 * \param[in] len Maximum size of the buffer #dst
 *
 * This copies at most #len - strlen(#dst) - 1 bytes from #src.
 *
 * \return length of concatenated string.
 */
size_t exoPal_strlcat(char* dst, const char* src, size_t len)
{
    return strlcat(dst, src, len);
}

/**@}*/

/*****************************************************************************
 * \defgroup Memory/NVRAM/Flash PAL
 * @{
 */

/*!
 * \brief Sets the cik
 *
 * Writes the 40 chars starting at cik* to nvm.
 *
 * \param[in] cik cik to write to nvm
 * \return errorcode if successful else 0
 */
uint8_t exoPal_setCik(const char * cik, uint8_t len)
{
    char zeroterminated[41];
    memcpy(zeroterminated, cik, MIN(40, len));
    zeroterminated[40] = '\0';
    setenv("INTGR8_CIK", zeroterminated, 1);
    return 0;
}

/*!
 * \brief Retrieves the stored CIK
 * \param[out] read_buffer pointer of buffer to place results in
 * \return 0 if successful, else errorCode
 */
uint8_t exoPal_getCik(char * read_buffer, uint8_t maxlen)
{
    char *env = getenv("INTGR8_CIK");
    strlcpy(read_buffer, env, maxlen);
    return 0;
}

/*!
 * \brief Retrieves the stored Model string
 * \param[in] read_buffer pointer of buffer to place results in
 * \return 0 if successful, else returns error code
 */
uint8_t exoPal_getModel(char * read_buffer, uint8_t maxlen)
{
    char *env = getenv("INTGR8_MODEL");
    strlcpy(read_buffer, env, maxlen);
    return 0;
}

/*!
 * \brief Retrieves the vendor string
 * \param[in] read_buffer pointer of buffer to place results in
 * \return returns 0 if successful, else returns error code.
 */
uint8_t exoPal_getVendor(char * read_buffer, uint8_t maxlen)
{
    char *env = getenv("INTGR8_VENDOR");
    strlcpy(read_buffer, env, maxlen);
    return 0;
}

/*!
 * \brief Retrieves UUID from device
 *
 *	This function retrieves a unique ID from your device.  This is typically
 *	the MEID of a cell modem, MAC address of a network card, or serial number
 *	of the device.
 *
 * \param[in] UUID_buf Buffer to put the devices UUID into.
 * \param[out]
 * \return 1 if failed to retrieve UUID, else 0
 */
uint8_t exoPal_getUuid(char * read_buffer, uint8_t maxlen)
{
    char *env = getenv("INTGR8_SN");
    strlcpy(read_buffer, env, maxlen);
    return 0;
}

/**@}*/
/*****************************************************************************
 * \defgroup Async Sockets PAL
 * @{
 */

/** \brief Setup the PAL state memory
 * \param[out] state The PAL state to initialize
 *
 * Typically, this is just writes zeros to the structure.
 */
void exoPal_init(exoPal_state_t *state)
{
    exoPal_memset(state, 0, sizeof(exoPal_state_t));
    state->polled.fd = -1;
}

/** \brief Setup PAL for IO
 * \param[in,out] state The PAL state.
 *
 * Any HW or SW initialization should be performed in here
 *
 * This function is meant to perform any one time initialization and/or setup.
 * This will be called everytime exosite_start is called.  It is expected that this
 * will do a DNS lookup of #host.
 *
 * When this is complete, it must call exoPal_ops_s::on_start_complete(). Either as
 * the last thing this function does, or sometime later from async activity.
 *
 * \retval 0 Everything is ok
 * \retval !0 Errors.
 */
int exoPal_start(exoPal_state_t *state, const char *host)
{

    struct hostent *resolved;
    resolved = gethostbyname(host);
    if (resolved == NULL) {
        fprintf(stderr, "XXX Failed to resolve host '%s' :%d:%s\n",
                host, h_errno, hstrerror(h_errno));
        return -1;
    }
    memcpy(&state->ip, resolved->h_addr, sizeof(state->ip));
    fprintf(stdout, "III Resolved '%s' to '%s'", host, inet_ntoa(state->ip));

    state->state = exoPal_state_initalized;
    if(state->ops.on_start_complete) {
        state->ops.on_start_complete(state, 0);
    }
    return 0;
}

/** \brief Close up any final bits.
 * \param[in,out] state The PAL state.
 * \return errorcode if failed, else a 0
 */
int exoPal_stop(exoPal_state_t *state)
{
    return 0;
}

/** \brief Opens a tcp socket
 * \param[in,out] state The PAL state.
 *
 * When this is complete, it must call exoPal_ops_s::on_connected(). Either as
 * the last thing this function does, or sometime later from async activity.
 *
 * \return errorcode if failed, else a 0
 */
int exoPal_tcpSocketOpen(exoPal_state_t *state)
{
    int fd, flags, ret;
    struct sockaddr_in addr;

    // FIXME: TLS.
    fd = socket(PF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        fprintf(stderr, "XXX Open socket failed %d:%s\n", errno, strerror(errno));
        return -1;
    }

    if((flags = fcntl(fd, F_GETFL, 0)) < 0) {
        fprintf(stderr, "XXX fcntl getflags failed %d:%s\n", errno, strerror(errno));
        shutdown(fd, SHUT_RDWR);
        return -1;
    }

    if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        fprintf(stderr, "XXX fcntl setflags failed %d:%s\n", errno, strerror(errno));
        shutdown(fd, SHUT_RDWR);
        return -1;
    }

    state->polled.fd = fd;
    state->polled.events = POLLIN | POLLOUT;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 443;
    addr.sin_addr = state->ip;

    ret = connect(state->polled.fd, (const struct sockaddr *)&addr, sizeof(addr));
    if(ret != 0 && errno != EINPROGRESS) {
        fprintf(stderr, "XXX connect failed %d:%s\n", errno, strerror(errno));
        shutdown(state->polled.fd, SHUT_RDWR);
        state->polled.fd = -1;
        return -1;
    }

    return 0;
}

/** \brief Closes a tcp socket
 * \param[in,out] state The PAL state.
 *
 * When this is complete, it must call exoPal_ops_s::on_socket_closed(). Either as
 * the last thing this function does, or sometime later from async activity.
 *
 * \return errorcode if failed, else a 0
 */
int exoPal_tcpSocketClose(exoPal_state_t *state)
{
    shutdown(state->polled.fd, SHUT_RDWR);
    state->polled.fd = -1;

    if(state->ops.on_socket_closed) {
        state->ops.on_socket_closed(state, 0);
    }
    return 0;
}

/** \brief Sends data to the open tcp socket
 * \param[in,out] state The PAL state.
 * \param[in] buffer Data to write to socket
 * \param[in] len Length of data to write to socket
 *
 * When this is complete, it must call exoPal_ops_s::on_send_complete(). Either as
 * the last thing this function does, or sometime later from async activity.
 *
 * \return errorcode if failed, else a 0
 */
int exoPal_socketWrite(exoPal_state_t *state, const char * buffer, uint16_t len)
{
    state->buf = (char*)buffer;
    state->buflen = len;
    state->state = exoPal_state_sending;
    state->polled.events = POLLOUT;

    return 0;
}


/** \brief Ask for some data
 * \param[in,out] state The PAL state.
 * \param[in] buffer Buffer received data will be written to
 * \param[in] bufferSize Size of buffer
 *
 * \note #buffer must remain and be untouched until the exoPal_ops_s::on_recv()
 * callback is called.
 *
 * When this is complete, it must call exoPal_ops_s::on_recv(). Either as
 * the last thing this function does, or sometime later from async activity.
 *
 * \return errorcode if failed, else a 0
 */
int exoPal_socketRead(exoPal_state_t *state, char * buffer, uint16_t bufSize)
{
    state->buf = buffer;
    state->buflen = bufSize;
    state->state = exoPal_state_recving;
    state->polled.events = POLLIN;

    return 0;
}

/** \brief  Used to do any operations before switching from sending to recving.
 * \param[in,out] state The PAL state.
 *
 * \return Returns 0 if successful, else error code
 */
int exoPal_sendingComplete(exoPal_state_t *state)
{
    return 0;
}

/**@}*/


int exoPal_processEvents(exoPal_state_t *state)
{
    int ret, opt_err;
    socklen_t err_size;

    ret = poll(&state->polled, 1, 10000);
    if(ret < 0) {
        fprintf(stderr, "XXX poll failed %d:%s\n", errno, strerror(errno));
        return ret;
    }

    switch(state->state) {
        case exoPal_state_connecting:
            if(state->polled.revents & POLLERR) {
                err_size = sizeof(opt_err);
                ret = getsockopt(state->polled.fd, SOL_SOCKET, SO_ERROR,
                        &opt_err, &err_size);
                if(ret != 0) {
                    opt_err = errno;
                }
                if(state->ops.on_connected) {
                    state->ops.on_connected(state, opt_err);
                }
            } else {
                if(state->ops.on_connected) {
                    state->ops.on_connected(state, 0);
                }
            }
            state->state = exoPal_state_initalized;
            break;

        case exoPal_state_sending:
            // writes ready.
            ret = send(state->polled.fd, state->buf, state->buflen, 0);
            if(ret < 0) {
                if(state->ops.on_send_complete) {
                    state->ops.on_send_complete(state, errno);
                }
                state->state = exoPal_state_initalized;
            } else {
                if(ret > state->buflen) ret = state->buflen;
                state->buflen -= ret;
                state->buf += ret;
                if(state->buflen > 0) {
                    // Didn't get it all.
                    state->polled.events = POLLOUT;

                } else {
                    if(state->ops.on_send_complete) {
                        state->ops.on_send_complete(state, 0);
                    }
                    state->state = exoPal_state_initalized;
                }
            }
            break;

        case exoPal_state_recving:
            ret = recv(state->polled.fd, state->buf, state->buflen, 0);
            if(ret < 0) {
                //ret = errno;
                fprintf(stderr, "XXX recv failed %d:%s\n", errno, strerror(errno));
            }
            if(state->ops.on_recv) {
                state->ops.on_recv(state, state->buf, ret);
            }
            state->state = exoPal_state_initalized;
            break;

        case exoPal_state_initalized:
        case exoPal_state_uninitalized:
            break;

    }

    return ret;
}

/* vim: set ai cin et sw=4 ts=4 : */
