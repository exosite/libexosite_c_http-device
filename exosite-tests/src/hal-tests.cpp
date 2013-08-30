#include "exosite_hal.c"
#include "gtest/gtest.h"

//
//class HalLibCleanNVM : public ::testing::Test {
//protected:
//virtual void SetUp() {
//    //reset nvm
//
//    mem.cik = "";
//    mem.model = "";
//    mem.server = "";
//    mem.uuid = "";
//    mem.vendor = "";
//
//}
//
//// virtual void TearDown() {}
//
//
//};
//
//
//// Checks the read and write CIK functions
//TEST_F(HalLibCleanNVM, cikRW)
//{
//    char testcik[41] = "abcdef1234abcdef1234abcdef1234abcdef1234";
//    exoHal_setCik(testcik);
//    char readResults[41] = "";
//    exoHal_getCik(readResults);
//    EXPECT_STREQ(testcik,readResults);
//    exoHAL_tcpSocketOpen();
//
//}
//
//
//// Check that a socket can be opened and some data received from Exosite
//TEST_F(HalLibCleanNVM, CommunicateWithExosite)
//{
//    uint8_t retVal=99;
//    retVal = exoHAL_tcpSocketOpen();
//    EXPECT_EQ(0,retVal);
//
//    retVal = exoHAL_socketWrite("GET /\r\n",sizeof("GET /\r\n"));
//    EXPECT_EQ(0,retVal);
//    
//    
//    char inBuf[100];
//    char * pBuf = inBuf;
//    uint16_t responseL=0;
//    retVal = exoHAL_socketRead(pBuf,100,&responseL);
//    EXPECT_EQ(0,retVal);
//    EXPECT_GT(responseL,0);
//
//    retVal = exoHAL_tcpSocketClose();
//    EXPECT_EQ(0,retVal);
//    int32_t t = 1;
//
//
//}
