#ifdef  __GNUC__
extern "C" {
#endif
#include "exosite.h"
#include "exosite_pal.h"
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
        nvm->retVal_setCik = 0;
        nvm->retVal_getCik = 0;
        nvm->retVal_getModel = 0;
        nvm->retVal_getVendor = 0;
        nvm->retVal_getUuid = 0;
        nvm->retVal_tcpSocketClose = 0;
        nvm->retVal_tcpSocketOpen = 0;
        nvm->retVal_socketRead = 0;
        nvm->retVal_socketWrite = 0;
    }

    struct UnitTest_storage * nvm;
    // virtual void TearDown() {}

};



// Checks the isCIKValid function, not the most robust, but should give
// a general idea
TEST_F(ExoLibCleanState, isCIKValid)
{
    char * testcik = "abcdef1234abcdef1234abcdef1234abcdef1234";
    uint8_t results = exosite_isCIKValid(testcik);
    EXPECT_EQ(1,results);

    // check if a non hexadecimal char causes check to fail
    testcik = "@abcdef1234abcdef1234abcdef1234abcdef124";
    results = exosite_isCIKValid(testcik);
    EXPECT_EQ(0,results);

    // check if a less then 40 char causes check to fail
    testcik = "abcdef1234abcdef1234abcdef1234abcdef123";
    results = exosite_isCIKValid(testcik);
    EXPECT_EQ(0,results);

    // cik may not be stored as a null terminated string, so we don't want to 
    // check if a more then 40 char causes check to fail.
    
}

// Make sure cik starts empty
TEST_F(ExoLibCleanState, cikStartEmpty)
{
     
    char testcik[41] = "abcdef1234abcdef1234abcdef1234abcdef1234";
    // make sure that we start up with an empty cik
    exosite_getCIK(testcik);
    EXPECT_STREQ("",testcik);
}

TEST_F(ExoLibCleanState, cikSetTest)
{
    // set cik 
    char testcik[41] = "abcdef1234abcdef1234abcdef1234abcdef1234";
    exosite_setCIK(testcik);

    // retrieve cik
    char newcik[41] = "";
    exosite_getCIK(newcik);

    // make sure they match
    EXPECT_STREQ(testcik, newcik);
}





TEST_F(ExoLibCleanState, sendRequest)
{
    strcpy(nvm->uuid,"123456789");
    char testcik[41] = "abcdef1234abcdef1234abcdef1234abcdef1234";
    exosite_setCIK(testcik);

    const char * vendor = "aVendor";
    const char * model = "aModel";
    exosite_init(vendor, model);
    
    message out_msg = {0};
    setMsg(&out_msg);

    parser_settings.on_header_field = header_field_cb;
    parser_settings.on_header_value = header_value_cb;
    parser_settings.on_body = body_cb;
    parser_settings.on_headers_complete = headers_complete_cb;
    parser_settings.on_url = request_url_cb;
    parser_settings.on_status_complete = status_complete_cb;

    
    http_parser_init(&parser, HTTP_REQUEST);
    
    size_t parsed;
    parsed = http_parser_execute(&parser, &parser_settings, nvm->writeToBuffer, nvm->writeToBufferLen);

    // Did parse correctly
    EXPECT_TRUE(parsed);

    // build expected body
    char expected_body[255] = {0};
    strcpy(expected_body,"vendor=");
    strcat(expected_body,vendor);
    strcat(expected_body,"&model=");
    strcat(expected_body,model);
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
