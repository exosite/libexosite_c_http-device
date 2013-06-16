cmake_minimum_required (VERSION 2.8)

project (exosite-c-libs )




find_package (Threads)


IF(MSVC)
# Need to compile files as CPP in order to get VS to "comply" with C99 stds
# required for gtest (http://stackoverflow.com/a/8274747/275354)
add_definitions("-D_VARIADIC_MAX=10")
ELSEIF(CMAKE_COMPILER_IS_GNUCC)
add_definitions(-std=c99)
ENDIF()

##########################exosite HAL######################################
add_subdirectory(exosite-hal)
###############################################################################


##########################exosite lib######################################
add_subdirectory(exosite-lib)
###############################################################################



##########################gtest lib######################################
add_subdirectory(gtest)
###############################################################################


##########################test runner code#####################################
add_subdirectory(testrunner)
###############################################################################

