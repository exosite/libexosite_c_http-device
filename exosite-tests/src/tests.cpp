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


TEST_F(ExoLibCleanNVM, cikRW)
{
    char testcik[41] = "abcdef1234abcdef1234abcdef1234abcdef1234";
    exoHAL_meta_write(testcik,40,EXO_META_CIK);   
    char readResults[41] = "";
    exoHAL_meta_read(readResults,EXO_META_CIK);
    EXPECT_STREQ(testcik,readResults);

    
}
