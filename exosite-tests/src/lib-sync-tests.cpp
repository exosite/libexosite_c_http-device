#ifdef  __GNUC__
extern "C" {
#endif
#include "exosite.h"
#include "exosite_pal.h"
#include "exosite_pal_private.h"
#include "http_parser.h"

#ifdef  __GNUC__
}
#endif

#include "msg.h"
#include "gtest/gtest.h"

http_parser parser;
static http_parser_settings parser_settings = {0};


#define TEST_MODEL "aModel"
#define TEST_VENDOR "aVendor"

char * validTestcik = (char*)"abcdef1234abcdef1234abcdef1234abcdef1234";
char * invalidTestcik_nonHexChar = (char*)"@abcdef1234abcdef1234abcdef1234abcdef124";
char * invalidTestcik_short = (char*)"abcdef1234abcdef1234abcdef1234abcdef123";
class ExoLibCleanState : public ::testing::Test
{
protected:
    virtual void SetUp() {
        //reset nvm
        nvm = (UnitTest_storage *)getUnitTestStorageStruct();
        nvm->cik[0] = '\0';
        nvm->uuid[0] = '\0';
        nvm->vendor[0] = '\0';
        nvm->model[0] = '\0';
        nvm->writeToBuffer[0] = '\0';
        nvm->readFromBuffer[0] = '\0';
        nvm->writeToBufferLen = 0;
        nvm->readFromBufferLen = 0;
        nvm->retVal_setCik = 0;
        nvm->retVal_getCik = 0;
        nvm->retVal_getModel = 0;
        nvm->retVal_getVendor = 0;
        nvm->retVal_getUuid = 0;
        nvm->retVal_tcpSocketClose = 0;
        nvm->retVal_tcpSocketOpen = 0;
        nvm->retVal_socketRead = 0;
        nvm->retVal_socketWrite = 0;

        parser_settings.on_header_field = header_field_cb;
        parser_settings.on_header_value = header_value_cb;
        parser_settings.on_body = body_cb;
        parser_settings.on_headers_complete = headers_complete_cb;
        parser_settings.on_url = request_url_cb;
        parser_settings.on_status_complete = status_complete_cb;
    }

    struct UnitTest_storage * nvm;
    // virtual void TearDown() {}

};



// Checks the isCIKValid function, not the most robust, but should give
// a general idea
TEST_F(ExoLibCleanState, isCIKValid)
{
    uint8_t results = exosite_isCIKValid(validTestcik);
    EXPECT_EQ(1,results);

    // check if a non hexadecimal char causes check to fail
    results = exosite_isCIKValid(invalidTestcik_nonHexChar);
    EXPECT_EQ(0,results);

    // check if a less then 40 char causes check to fail
    results = exosite_isCIKValid(invalidTestcik_short);
    EXPECT_EQ(0,results);

    // cik may not be stored as a null terminated string, so we don't want to
    // check if a more then 40 char causes check to fail.

}

// Make sure cik starts empty
TEST_F(ExoLibCleanState, cikStartEmpty)
{

    char emptyBuffer[41];
    // make sure that we start up with an empty cik
    exosite_getCIK(emptyBuffer);
    EXPECT_STREQ("",emptyBuffer);
}

TEST_F(ExoLibCleanState, cikSetTest)
{
    // set cik
    exosite_setCIK(validTestcik);

    // retrieve cik
    char newcik[41] = "";
    exosite_getCIK(newcik);

    // make sure they match
    EXPECT_STREQ(validTestcik, newcik);
}





TEST_F(ExoLibCleanState, provisionActivateRequest)
{
    strcpy(nvm->uuid,"123456789");
    exosite_setCIK(validTestcik);


    EXO_STATE resp;
    resp = exosite_init(TEST_VENDOR, TEST_MODEL);

    // We didn't set a response, so we should receive a NO_RESPONSE
    EXPECT_EQ(EXO_STATE_NO_RESPONSE,resp);

    message out_msg = {0};
    setMsg(&out_msg);

    http_parser_init(&parser, HTTP_REQUEST);

    size_t parsed;
    parsed = http_parser_execute(&parser, &parser_settings, nvm->writeToBuffer, nvm->writeToBufferLen);

    // Did parse correctly
    EXPECT_TRUE(parsed);

    // build expected body
    char expected_body[255] = {0};
    strcpy(expected_body,"vendor=");
    strcat(expected_body,TEST_VENDOR);
    strcat(expected_body,"&model=");
    strcat(expected_body,TEST_MODEL);
    strcat(expected_body,"&sn=");
    strcat(expected_body,nvm->uuid);


    // Check body is what we want
    EXPECT_STREQ(expected_body,out_msg.body);

    // check request url is correct
    EXPECT_STREQ("/provision/activate", out_msg.request_url);

    // Check Content Length is correct
    uint16_t body_length = strlen(expected_body);
    char contentLengthStr[5];
    exoPal_itoa(body_length,contentLengthStr,5);
    EXPECT_STREQ( contentLengthStr, out_msg.headers[2][1]);
    EXPECT_EQ( body_length, out_msg.body_size);
}

TEST_F(ExoLibCleanState, provisionActivate_200Response_goodFormat)
{

    strcpy(nvm->readFromBuffer,"HTTP/1.1 200 OK\r\nContent-Length: 40\r\n\r\n");
    strcat(nvm->readFromBuffer,validTestcik);

    nvm->readFromBufferLen = strlen(nvm->readFromBuffer);
    // we received a 200 response, which means we received a valid cik
    strcpy(nvm->uuid,"123456789");


    EXO_STATE resp;
    resp = exosite_init(TEST_VENDOR, TEST_MODEL);

    // Init should have been successful
    EXPECT_EQ(EXO_STATE_INIT_COMPLETE,resp);

    // retrieve cik
    char newcik[41] = "";
    exosite_getCIK(newcik);

    // We should have our new cik
    EXPECT_STREQ(validTestcik, newcik);

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
// vim: set ai cin et sw=4 ts=4 :
