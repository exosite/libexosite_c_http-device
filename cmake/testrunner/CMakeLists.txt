include_directories(
  ../../gtest-1.6.0/include/gtest
  ../../gtest-1.6.0/include
  ../../gtest-1.6.0/src
  ../../exosite-lib/include/
  ../../exosite-lib/src
)

set(test_srcs
    ../../exosite-tests/src/main.cpp
    ../../exosite-tests/src/tests.cpp
    ../../exosite-lib/include/exosite_hal.h
)

IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${test_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_executable(testrunner ${test_srcs} )

set_target_properties (testrunner PROPERTIES COMPILE_DEFINITIONS "TESTING")

target_link_libraries(testrunner exosite-lib gtest)