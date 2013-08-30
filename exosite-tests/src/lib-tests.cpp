#include "exosite.c"
#include "exosite_hal.h"
#include "gtest/gtest.h"


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
    uint8_t results = Exosite_isCIKValid(testcik);
    EXPECT_EQ(1,results);

    // check if a non hexadecimal char causes check to fail
    testcik = "@abcdef1234abcdef1234abcdef1234abcdef124";
    results = Exosite_isCIKValid(testcik);
    EXPECT_EQ(0,results);

    // check if a less then 40 char causes check to fail
    testcik = "abcdef1234abcdef1234abcdef1234abcdef123";
    results = Exosite_isCIKValid(testcik);
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
    char testcik[41] = "abcdef1234abcdef1234abcdef1234abcdef1234";
    // make sure that we start up with an empty cik
    exosite_setCIK(testcik);

    char newcik[41] = "";
    // make sure that we start up with an empty cik
    exosite_getCIK(newcik);

    EXPECT_STREQ(testcik, newcik);
}
