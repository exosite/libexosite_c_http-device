cmake_minimum_required (VERSION 2.8)

project (exosite-c-libs )


include_directories(
  ../gtest-1.6.0/include/gtest
  ../gtest-1.6.0/include
  ../gtest-1.6.0/src
  ../exosite-lib/include/
)

find_package (Threads)


IF(MSVC)
# Need to compile files as CPP in order to get VS to "comply" with C99 stds
# required for gtest (http://stackoverflow.com/a/8274747/275354)
add_definitions("-D_VARIADIC_MAX=10")
ELSEIF(CMAKE_COMPILER_IS_GNUCC)
add_definitions(-std=c99)
ENDIF()

##########################exosite HAL######################################
set(hal_srcs
   ../exosite-lib/src/exosite_hal.c
   ../exosite-lib/src/exosite_meta.c
   ../exosite-lib/include/exosite_hal.h
   ../exosite-lib/include/exosite_meta.h

)

# Need to compile files as CPP in order to get VS to "comply" with C99 stds
IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${hal_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_library(exosite-hal STATIC ${hal_srcs})
###############################################################################


##########################exosite lib######################################
set(lib_srcs
    ../exosite-lib/src/exosite.c
    ../exosite-lib/include/exosite.h
)

IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${lib_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_library(exosite-lib STATIC ${lib_srcs})

target_link_libraries(exosite-lib exosite-hal gtest)
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
    ../exosite-tests/src/main.cpp
    ../exosite-tests/src/exosite_hal.c
    ../exosite-tests/src/exosite_meta.c
    ../exosite-lib/include/exosite_hal.h
    ../exosite-lib/include/exosite_meta.h
)

add_executable(tests ${test_srcs} ${lib_srcs})

target_link_libraries(tests exosite-lib gtest)
###############################################################################

