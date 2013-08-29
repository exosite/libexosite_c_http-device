#include "exosite_hal.c"
#include "gtest/gtest.h"


class ExoLibCleanNVM : public ::testing::Test {
protected:
virtual void SetUp() {
    //reset nvm

    mem.cik = "";
    mem.model = "";
    mem.server = "";
    mem.uuid = "";
    mem.vendor = "";

}

// virtual void TearDown() {}


};


// Checks the read and write CIK functions
TEST_F(ExoLibCleanNVM, cikRW)
{
    char testcik[41] = "abcdef1234abcdef1234abcdef1234abcdef1234";
    exoHal_setCik(testcik);
    char readResults[41] = "";
    exoHal_getCik(readResults);
    EXPECT_STREQ(testcik,readResults);


}
//
//
//// Checks that the nvmClear actually clears out NVM
//TEST_F(ExoLibCleanNVM, nvmClear)
//{
//    mem.cik = "abcdef1234abcdef1234abcdef1234abcdef1234";
//    mem.model = "myModelName";
//    mem.server = "myServerName";
//    mem.uuid = "aUUID";
//    mem.vendor = "myVendorName";
//
//
//    exoHAL_EraseMeta();
//    
//    EXPECT_STREQ(mem.cik, "");
//    EXPECT_STREQ(mem.model, "");
//    EXPECT_STREQ(mem.server, "");
//    EXPECT_STREQ(mem.uuid, "");
//    EXPECT_STREQ(mem.vendor, "");
//    
//
//
//}
