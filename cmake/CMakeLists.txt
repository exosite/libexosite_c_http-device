cmake_minimum_required (VERSION 2.8)

project (exosite-c-libs )


add_compile_options(-g)


find_package (Threads)


IF(MSVC)
# Need to compile files as CPP in order to get VS to "comply" with C99 stds
# required for gtest (http://stackoverflow.com/a/8274747/275354)
add_definitions("-D_VARIADIC_MAX=10")
ELSEIF(CMAKE_COMPILER_IS_GNUCC)
add_definitions(-std=c99)
ENDIF()

##########################exosite PAL######################################
add_subdirectory(exosite-pal)

##########################exosite lib######################################
add_subdirectory(exosite-lib)

##########################docs#############################################
add_subdirectory(docs)

##########################gtest lib########################################
add_subdirectory(gtest)

##########################test runner code#################################
add_subdirectory(testrunner)

add_subdirectory(main)

#  vim: set sw=4 ts=4 :
