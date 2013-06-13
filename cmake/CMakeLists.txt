cmake_minimum_required (VERSION 2.8)

project (exosite-c-libs )

project (exosite-c-libs)

include_directories(
  ../gtest-1.6.0/include/gtest
  ../gtest-1.6.0/include
  ../gtest-1.6.0/src
)

find_package (Threads)

# required for gtest (http://stackoverflow.com/a/8274747/275354)
add_definitions("-D_VARIADIC_MAX=10")

add_definitions(-std=c99)

##########################exosite lib######################################
set(lib_srcs
    ../src/exosite.c
    ../src/exosite_hal.c
    ../src/exosite_meta.c
)

# Need to compile files as CPP in order to get VS to "comply" with C99 stds
IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${lib_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_library(exosite-lib STATIC ${lib_srcs})
###############################################################################


##########################gtest lib######################################
set(gtest_srcs
    ../gtest-1.6.0/src/gtest-all.cc
)

add_library(gtest STATIC ${gtest_srcs})
target_link_libraries(gtest ${CMAKE_THREAD_LIBS_INIT})

###############################################################################


##########################test runner code#####################################
set(test_srcs
    ../src-tests/main.cpp
    ../src-tests/tests.cpp
)

add_executable(tests ${test_srcs})

target_link_libraries(tests exosite-lib gtest)
###############################################################################

