#include "exosite.c"
#include "gtest/gtest.h"


class ExoLibCleanNVM : public ::testing::Test {
protected:
    virtual void SetUp() {
        //reset nvm
        exoHAL_EraseMeta();

    }

    // virtual void TearDown() {}


};



// Checks the read and write CIK functions
TEST_F(ExoLibCleanNVM, isCIKValid)
{
    char testcik[41] = "abcdef1234abcdef1234abcdef1234abcdef1234";
    uint8_t results = Exosite_isCIKValid(testcik);
    EXPECT_EQ(1,results);
    



}
