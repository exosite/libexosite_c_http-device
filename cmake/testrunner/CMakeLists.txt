include_directories(
  ../../gtest-1.6.0/include/gtest
  ../../gtest-1.6.0/include
  ../../gtest-1.6.0/src
  ../../exosite-lib/include/
  ../../exosite-pal/include/
  ../../exosite-pal/unit_tests/include/
  ../../exosite-tests/include/
)

set(test_srcs
    ../../exosite-tests/src/main.cpp
    ../../exosite-tests/src/msg.cpp
    ../../exosite-tests/src/http_parser.c
    ../../exosite-tests/src/pal-tests.cpp
    ../../exosite-tests/src/lib-tests.cpp
    ../../exosite-tests/include/msg.h
    ../../exosite-tests/include/http_parser.h
    ../../exosite-pal/include/exosite_pal.h
    ../../exosite-lib/include/exosite.h
)

IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${test_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_executable(testrunner ${test_srcs} )

set_target_properties (testrunner PROPERTIES COMPILE_DEFINITIONS "TESTING")

target_link_libraries(testrunner exosite-pal exosite-lib gtest)
