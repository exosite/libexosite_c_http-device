#ifdef  __GNUC__
extern "C" {
#endif
#include "exosite_async.h"
#include "exosite_pal_async.h"
#include "http_parser.h"

#ifdef  __GNUC__
}
#endif

#include "msg.h"
#include "gtest/gtest.h"


#define TEST_MODEL "aModel"
#define TEST_VENDOR "aVendor"


class ExositeAsyncLib : public ::testing::Test
{
public:
    uint8_t callbacksHit;
    int hit_cb_start_complete;
    int hit_cb_write_complete;
    int hit_cb_read_begin;
    int hit_cb_read_raw;
    int hit_cb_read_complete;
    int hit_cb_timestamp_complete;
protected:
    http_parser parser;
    http_parser_settings parser_settings;


    Exosite_state_t exoLib;
    exoPal_state_t exoPal;

    struct UnitTest_storage * nvm;

    virtual void SetUp();
    // virtual void TearDown() {}
};

/*
 * Failme callbacks.  Use this callback for the ones that should NOT get called
 * during a test.
 */
#define CB_BIT_start_complete     (1<<0)
#define CB_BIT_write_complete     (1<<1)
#define CB_BIT_read_begin         (1<<2)
#define CB_BIT_read_raw           (1<<3)
#define CB_BIT_read_complete      (1<<4)
#define CB_BIT_timestamp_complete (1<<5)
int exotest_failme_start_complete(Exosite_state_t *state, int status)
{
    EXPECT_TRUE(false);
    ExositeAsyncLib *me = (ExositeAsyncLib *)state->context;
    me->callbacksHit |= CB_BIT_start_complete;
    me->hit_cb_start_complete ++;
    return -1;
}
int exotest_failme_write_complete(Exosite_state_t *state, int status)
{
    EXPECT_TRUE(false);
    ExositeAsyncLib *me = (ExositeAsyncLib *)state->context;
    me->callbacksHit |= CB_BIT_write_complete;
    me->hit_cb_write_complete ++;
    return -1;
}
int exotest_failme_read_begin(Exosite_state_t *state, int status)
{
    EXPECT_TRUE(false);
    ExositeAsyncLib *me = (ExositeAsyncLib *)state->context;
    me->callbacksHit |= CB_BIT_read_begin;
    me->hit_cb_read_begin ++;
    return -1;
}
int exotest_failme_read_raw(Exosite_state_t *state, const char *data, size_t len)
{
    EXPECT_TRUE(false);
    ExositeAsyncLib *me = (ExositeAsyncLib *)state->context;
    me->callbacksHit |= CB_BIT_read_raw;
    me->hit_cb_read_raw ++;
    return -1;
}
int exotest_failme_read_complete(Exosite_state_t *state, int status)
{
    EXPECT_TRUE(false);
    ExositeAsyncLib *me = (ExositeAsyncLib *)state->context;
    me->callbacksHit |= CB_BIT_read_complete;
    me->hit_cb_read_complete ++;
    return -1;
}
int exotest_failme_timestamp_complete(Exosite_state_t *state, const char *data, size_t len)
{
    EXPECT_TRUE(false);
    ExositeAsyncLib *me = (ExositeAsyncLib *)state->context;
    me->callbacksHit |= CB_BIT_timestamp_complete;
    me->hit_cb_timestamp_complete ++;
    return -1;
}

void ExositeAsyncLib::SetUp() {
    //reset nvm
    nvm = (UnitTest_storage *)getUnitTestStorageStruct();
    resetUnitTestStorageStruct();

    parser_settings.on_header_field = header_field_cb;
    parser_settings.on_header_value = header_value_cb;
    parser_settings.on_body = body_cb;
    parser_settings.on_headers_complete = headers_complete_cb;
    parser_settings.on_url = request_url_cb;
    parser_settings.on_status_complete = status_complete_cb;

    callbacksHit = 0;
    hit_cb_start_complete = 0;
    hit_cb_write_complete = 0;
    hit_cb_read_begin = 0;
    hit_cb_read_raw = 0;
    hit_cb_read_complete = 0;
    hit_cb_timestamp_complete = 0;

    exosite_init(&exoLib);
    exoLib.exoPal = &exoPal;
    exoLib.context = this;
    exoLib.ops.on_start_complete = exotest_failme_start_complete;
    exoLib.ops.on_write_complete = exotest_failme_write_complete;
    exoLib.ops.on_read_begin = exotest_failme_read_begin;
    exoLib.ops.on_read_raw = exotest_failme_read_raw;
    exoLib.ops.on_read_complete = exotest_failme_read_complete;
    exoLib.ops.on_timestamp_complete = exotest_failme_timestamp_complete;

}


/******************************************************************************/
int exotest_canNotStart_status(Exosite_state_t *state, int status)
{
    ExositeAsyncLib *me = (ExositeAsyncLib *)state->context;
    me->callbacksHit |= CB_BIT_start_complete;
    me->hit_cb_start_complete ++;
    EXPECT_EQ(100, status);
    return -1;
}
TEST_F(ExositeAsyncLib, canNotStart)
{
    int ret;
    exoLib.ops.on_start_complete = exotest_canNotStart_status;

    // Setup test data
    strlcpy(nvm->vendor, TEST_VENDOR, sizeof(nvm->vendor));
    strlcpy(nvm->uuid, "1234567", sizeof(nvm->uuid));
    nvm->retVal_start = 100;

    // Call function to test
    ret = exosite_start(&exoLib);
    // This will call:
    // - exoPal_init()
    // - exoPal_getVendor()
    // - exoPal_getUuid()
    // - exoPal_start()
    //   - IF status != 0; does NOT call activate, and done.

    EXPECT_EQ(100, ret);
    EXPECT_STREQ(TEST_VENDOR, exoLib.projectid);
    EXPECT_STREQ("1234567", exoLib.uuid);
    EXPECT_STREQ("aVendor.m2.exosite.com", nvm->hostname);
    EXPECT_EQ(CB_BIT_start_complete, callbacksHit);
    EXPECT_EQ(1, hit_cb_start_complete);
}

/******************************************************************************/
int exotest_canStart_status(Exosite_state_t *state, int status)
{
    ExositeAsyncLib *me = (ExositeAsyncLib *)state->context;
    me->callbacksHit |= CB_BIT_start_complete;
    me->hit_cb_start_complete ++;
    EXPECT_EQ(200, status);
    return -1;
}
TEST_F(ExositeAsyncLib, canStart)
{
    int ret;
    exoLib.ops.on_start_complete = exotest_canStart_status;

    // Setup test data
    strlcpy(nvm->vendor, TEST_VENDOR, sizeof(nvm->vendor));
    strlcpy(nvm->uuid, "1234567", sizeof(nvm->uuid));
    nvm->retVal_start = 0;
    nvm->retVal_tcpSocketOpen = 0;
    nvm->retVal_socketWrite = 0;
    nvm->retVal_socketRead = 0;
    nvm->retVal_tcpSocketClose = 0;
    nvm->readFromBufferLen = strlcpy(nvm->readFromBuffer, "HTTP/1.1 200 OK\r\n"
            "Server: faker\r\n"
            "Connection: Keep-Alive\r\n"
            "Content-Length: 40\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "\r\n"
            "abcdef1234abcdef1234abcdef1234abcdef1234"
            , sizeof(nvm->readFromBuffer));

    // Call function to test
    ret = exosite_start(&exoLib);
    // This will call:
    // - exoPal_init()
    // - exoPal_getVendor()
    // - exoPal_getUuid()
    // - exoPal_start()
    // - exosite_activate()
    // - exoPal_tcpSocketOpen()
    // - mutilple: exoPal_socketWrite()
    // - exoPal_sendingComplete()
    // - as many as needed: exoPal_socketRead()
    // - exoPal_setCik()
    // - exoPal_tcpSocketClose()
    // - ops.on_start_complete()

    EXPECT_EQ(0, ret);
    EXPECT_STREQ(TEST_VENDOR, exoLib.projectid);
    EXPECT_STREQ("1234567", exoLib.uuid);
    EXPECT_STREQ("aVendor.m2.exosite.com", nvm->hostname);
    EXPECT_STREQ("abcdef1234abcdef1234abcdef1234abcdef1234", exoLib.cik);
    EXPECT_STREQ("POST /provision/activate HTTP/1.1\r\n"
            "Host: aVendor.m2.exosite.com\r\n"
            "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n"
            "Accept: application/x-www-form-urlencoded; charset=utf-8\r\n"
            "Content-Length: 39\r\n"
            "\r\n"
            "vendor=aVendor&model=aVendor&sn=1234567",
            nvm->writeToBuffer);
    EXPECT_EQ(CB_BIT_start_complete, callbacksHit);
    EXPECT_EQ(1, hit_cb_start_complete);
}

int exotest_writeRequest_status(Exosite_state_t *state, int status)
{
    ExositeAsyncLib *me = (ExositeAsyncLib *)state->context;
    me->callbacksHit |= CB_BIT_write_complete;
    me->hit_cb_write_complete ++;
    EXPECT_EQ(204, status);
    return -1;
}
TEST_F(ExositeAsyncLib, writeRequest)
{
    int ret;
    // Setup test data
    strlcpy(nvm->vendor, TEST_VENDOR, sizeof(nvm->vendor));
    strlcpy(nvm->uuid, "1234567", sizeof(nvm->uuid));
    nvm->retVal_start = 0;
    nvm->retVal_tcpSocketOpen = 0;
    nvm->retVal_socketWrite = 0;
    nvm->retVal_socketRead = 0;
    nvm->retVal_tcpSocketClose = 0;
    nvm->readFromBufferLen = strlcpy(nvm->readFromBuffer, "HTTP/1.1 204 No Content\r\n"
            "Server: faker\r\n"
            "Connection: Close\r\n"
            "Content-Length: 0\r\n"
            "\r\n"
            , sizeof(nvm->readFromBuffer));

    // 'Mock' that exosite_start() was called.
    strlcpy(exoLib.cik, "abcdef1234abcdef1234abcdef1234abcdef1234", sizeof(exoLib.cik));
    strlcpy(exoLib.projectid, TEST_VENDOR, sizeof(exoLib.projectid));
    strlcpy(exoLib.uuid, "1234567", sizeof(exoLib.uuid));
    exoLib.state = Exosite_State_idle;
    exoLib.stage = Exosite_Stage_idle;
    exoLib.ops.on_write_complete = exotest_writeRequest_status;

    // Call function to test
    ret = exosite_write(&exoLib, "temp=42&hum=30&lux=14000");
    // This will call:
    // - exoPal_tcpSocketOpen()
    // - mutilple: exoPal_socketWrite()
    // - exoPal_sendingComplete()
    // - as many as needed: exoPal_socketRead()
    // - exoPal_tcpSocketClose()
    // - ops.on_write_complete()

    EXPECT_EQ(0, ret);
    EXPECT_STREQ(
            "POST /onep:v1/stack/alias HTTP/1.1\r\n"
            "Host: aVendor.m2.exosite.com\r\n"
            "X-Exosite-CIK: abcdef1234abcdef1234abcdef1234abcdef1234\r\n"
            "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n"
            "Accept: application/x-www-form-urlencoded; charset=utf-8\r\n"
            "Content-Length: 24\r\n"
            "\r\n"
            "temp=42&hum=30&lux=14000",
            nvm->writeToBuffer);
    EXPECT_EQ(CB_BIT_write_complete, callbacksHit);
    EXPECT_EQ(1, hit_cb_write_complete);

}

#if 0
TEST_F(ExoLibCleanState, writeRequest)
{
    // Checks that the write request is properly formatted
    exosite_setCIK(validTestcik);


    EXO_STATE respa;
    respa = exosite_init(TEST_VENDOR, TEST_MODEL);

    // We didn't set a response, so we should receive a NO_RESPONSE
    EXPECT_EQ(EXO_STATE_NO_RESPONSE,respa);

    uint8_t respR;

    message out_msg = {0};
    setMsg(&out_msg);

    http_parser_init(&parser, HTTP_REQUEST);


    strcpy(nvm->readFromBuffer,"HTTP/1.1 200 OK\r\nsome random header = blah\r\n\r\n");

    const char * writeInfo = "testAlias=4";
    uint16_t resLength = 0;
    respR = exosite_write(writeInfo,  strlen(writeInfo));


    // Parse write request
    size_t parsed;
    parsed = http_parser_execute(&parser, &parser_settings, nvm->writeToBuffer, nvm->writeToBufferLen);

    // check cik is correct
    EXPECT_STREQ(validTestcik,out_msg.headers[1][1]);

    // Check body is what we want
    EXPECT_STREQ(writeInfo,out_msg.body);

    // check request url is correct
    EXPECT_STREQ("/onep:v1/stack/alias", out_msg.request_url);

    // Check body size is correct
    uint16_t body_length = strlen(writeInfo);
    EXPECT_EQ( body_length, out_msg.body_size);

    // check that content length header is same as our measured body length
    EXPECT_EQ(body_length, atoi(out_msg.headers[3][1]));
}
TEST_F(ExoLibCleanState, readRequest)
{
    // Checks that the read request is properly formatted
    strcpy(nvm->uuid,"123456789");
    exosite_setCIK(validTestcik);


    EXO_STATE respa;
    respa = exosite_init(TEST_VENDOR, TEST_MODEL);

    // We didn't set a response, so we should receive a NO_RESPONSE
    EXPECT_EQ(EXO_STATE_NO_RESPONSE,respa);

    uint8_t respR;

    message out_msg = {0};
    setMsg(&out_msg);

    http_parser_init(&parser, HTTP_REQUEST);


    strcpy(nvm->readFromBuffer,"HTTP/1.1 200 OK\r\nsome random header = blah\r\n\r\ntestAlias=123&anotherAlias=456");
    nvm->readFromBufferLen = strlen("HTTP/1.1 200 OK\r\nsome random header = blah\r\n\r\ntestAlias=123&anotherAlias=456");

    char responseBuffer[128] = {'\0'};
    uint16_t resLength = 0;
    respR = exosite_read("testAlias&anotherAlias", responseBuffer,128, &resLength);

    // Check response body is correct.
    EXPECT_EQ(strlen("testAlias=123&anotherAlias=456"), resLength);
    EXPECT_STREQ("testAlias=123&anotherAlias=456", responseBuffer);


    // build expected body
    char expected_body[255] = {0};
    strcpy(expected_body,"");

    size_t parsed;
    parsed = http_parser_execute(&parser, &parser_settings, nvm->writeToBuffer, nvm->writeToBufferLen);

    // check cik is correct
    EXPECT_STREQ(validTestcik, out_msg.headers[1][1]);

    // Check body is what we want
    EXPECT_STREQ(expected_body, out_msg.body);

    // check request url is correct
    EXPECT_STREQ("/onep:v1/stack/alias?testAlias&anotherAlias", out_msg.request_url);

    // Check body size is correct
    uint16_t body_length = strlen(expected_body);
    EXPECT_EQ( body_length, out_msg.body_size);
}

TEST_F(ExoLibCleanState, read_200Response_goodFormat)
{
    // checks that we properly process a valid read response
    strcpy(nvm->uuid,"123456789");
    exosite_setCIK(validTestcik);


    uint8_t resp;
    resp = exosite_init(TEST_VENDOR, TEST_MODEL);

    // We didn't set a response, so we should receive a NO_RESPONSE
    EXPECT_EQ(EXO_STATE_NO_RESPONSE,resp);

    message out_msg = {0};
    setMsg(&out_msg);

    http_parser_init(&parser, HTTP_REQUEST);

    const char * xoResponse = "HTTP/1.1 200 OK\r\nsome random header = blah\r\n\r\ntestAlias=5&anotherAlias=here";
    strcpy(nvm->readFromBuffer, xoResponse);

    nvm->readFromBufferLen = strlen(xoResponse);

    char responseBuffer[128] = {'\0'};
    uint16_t resLength = 0;
    resp = exosite_read("testAlias&anotherAlias", responseBuffer,128, &resLength);

      // Check response body is correct.
    EXPECT_EQ(strlen("testAlias=5&anotherAlias=here"), resLength);
    EXPECT_STREQ("testAlias=5&anotherAlias=here", responseBuffer);



    size_t parsed;
    parsed = http_parser_execute(&parser, &parser_settings, nvm->writeToBuffer, nvm->writeToBufferLen);

    // Did parse correctly
    EXPECT_TRUE(parsed);

    // build expected body
    char * expected_response = (char*)"testAlias=5&anotherAlias=here";

    // Check response is the value we responded with
    EXPECT_STREQ(expected_response,responseBuffer);

}

TEST_F(ExoLibCleanState, readSingleRequest)
{
    // Checks that the read request is properly formatted
    strcpy(nvm->uuid,"123456789");
    exosite_setCIK(validTestcik);


    EXO_STATE respa;
    respa = exosite_init(TEST_VENDOR, TEST_MODEL);

    // We didn't set a response, so we should receive a NO_RESPONSE
    EXPECT_EQ(EXO_STATE_NO_RESPONSE,respa);

    uint8_t respR;

    message out_msg = {0};
    setMsg(&out_msg);

    http_parser_init(&parser, HTTP_REQUEST);


    strcpy(nvm->readFromBuffer,"HTTP/1.1 200 OK\r\nsome random header = blah\r\n\r\n");

    char responseBuffer[128] = {'\0'};
    uint16_t resLength = 0;
    respR = exosite_read("testAlias", responseBuffer,128, &resLength);

    // build expected body
    char expected_body[255] = {0};
    strcpy(expected_body,"");

    size_t parsed;
    parsed = http_parser_execute(&parser, &parser_settings, nvm->writeToBuffer, nvm->writeToBufferLen);

    // check cik is correct
    EXPECT_STREQ(validTestcik,out_msg.headers[1][1]);

    // Check body is what we want
    EXPECT_STREQ(expected_body,out_msg.body);

    // check request url is correct
    EXPECT_STREQ("/onep:v1/stack/alias?testAlias", out_msg.request_url);

    // Check body size is correct
    uint16_t body_length = strlen(expected_body);
    EXPECT_EQ( body_length, out_msg.body_size);
}

TEST_F(ExoLibCleanState, readSingle_200Response_goodFormat)
{
    // checks that we properly process a valid read response
    strcpy(nvm->uuid,"123456789");
    exosite_setCIK(validTestcik);

    uint8_t resp;
    resp = exosite_init(TEST_VENDOR, TEST_MODEL);

    // We didn't set a response, so we should receive a NO_RESPONSE
    EXPECT_EQ(EXO_STATE_NO_RESPONSE,resp);

    message out_msg = {0};
    setMsg(&out_msg);

    http_parser_init(&parser, HTTP_REQUEST);

    const char * xoResponse = "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\ntestAlias=5";
    strcpy(nvm->readFromBuffer, xoResponse);

    nvm->readFromBufferLen = strlen(xoResponse);

    char responseBuffer[128] = {'\0'};
    uint16_t resLength = 0;
    resp = exosite_readSingle("testAlias", responseBuffer,128, &resLength);



    size_t parsed;
    parsed = http_parser_execute(&parser, &parser_settings, nvm->writeToBuffer, nvm->writeToBufferLen);

    // Did parse correctly
    EXPECT_TRUE(parsed);

    // build expected body
    char * expected_response = (char*)"5";

    // Check response is the value we responded with
    EXPECT_STREQ(expected_response,responseBuffer);

}

#endif
// vim: set ai cin et sw=4 ts=4 :
