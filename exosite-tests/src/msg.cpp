#include "msg.h"
#include "stdio.h"
#include "string.h"

message *curMsg = 0;

void setMsg(message * msg)
{
    curMsg = msg;
}




size_t strlncat(char *dst, size_t len, const char *src, size_t n)
{
    size_t slen;
    size_t dlen;
    size_t rlen;
    size_t ncpy;

    slen = strnlen(src, n);
    dlen = strnlen(dst, len);

    if (dlen < len)
    {
        rlen = len - dlen;
        ncpy = slen < rlen ? slen : (rlen - 1);
        memcpy(dst + dlen, src, ncpy);
        dst[dlen + ncpy] = '\0';
    }

    return slen + dlen;
}

int status_complete_cb (http_parser *p) {
    // p.data++;
    return 0;
}

int header_field_cb (http_parser *p, const char *buf, size_t len)
{



    if (curMsg->last_header_element != FIELD)
        curMsg->num_headers++;

    strlncat(curMsg->headers[curMsg->num_headers-1][0],
             sizeof(curMsg->headers[curMsg->num_headers-1][0]),
             buf,
             len);

    curMsg->last_header_element = FIELD;

    return 0;
}

int header_value_cb (http_parser *p, const char *buf, size_t len)
{



    strlncat(curMsg->headers[curMsg->num_headers-1][1],
             sizeof(curMsg->headers[curMsg->num_headers-1][1]),
             buf,
             len);

    curMsg->last_header_element = VALUE;

    return 0;
}

void check_body_is_final (const http_parser *p)
{
    if (curMsg->body_is_final)
    {
        fprintf(stderr, "\n\n *** Error http_body_is_final() should return 1 "
                "on last on_body callback call "
                "but it doesn't! ***\n\n");
        //abort();
    }
    curMsg->body_is_final = http_body_is_final(p);
}

int body_cb (http_parser *p, const char *buf, size_t len)
{
    strlncat(curMsg->body,
             sizeof(curMsg->body),
             buf,
             len);
    curMsg->body_size += len;
    check_body_is_final(p);
    // printf("body_cb: '%s'\n", requests[num_messages].body);
    return 0;
}

int count_body_cb (http_parser *p, const char *buf, size_t len)
{
    curMsg->body_size += len;
    check_body_is_final(p);
    return 0;
}

int request_url_cb (http_parser *p, const char *buf, size_t len)
{
    strlncat(curMsg->request_url,
             sizeof(curMsg->request_url),
             buf,
             len);
    return 0;
}



int headers_complete_cb (http_parser *p)
{
    curMsg->method = (http_method)p->method;
    curMsg->status_code = p->status_code;
    curMsg->http_major = p->http_major;
    curMsg->http_minor = p->http_minor;
    curMsg->headers_complete_cb_called = TRUE;
    curMsg->should_keep_alive = http_should_keep_alive(p);
    return 0;
}
