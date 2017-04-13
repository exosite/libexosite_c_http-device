include_directories(
	../../gtest-1.6.0/include/gtest
	../../gtest-1.6.0/include
	../../gtest-1.6.0/src
	../../exosite-lib/include/
	../../exosite-pal/unit_tests_sync/include/
	../../exosite-pal/unit_tests_async/include/
	../../exosite-tests/include/
)

set(test_sync_srcs
	../../exosite-tests/src/main.cpp
	../../exosite-tests/src/msg.cpp
	../../exosite-tests/src/http_parser.c
	../../exosite-tests/src/lib-sync-tests.cpp
	../../exosite-tests/include/msg.h
	../../exosite-tests/include/http_parser.h
	../../exosite-pal/unit_tests_sync/include/exosite_pal.h
	../../exosite-pal/unit_tests_sync/include/exosite_pal_private.h
	../../exosite-lib/include/exosite.h
)

set(test_async_srcs
	../../exosite-tests/src/main.cpp
	../../exosite-tests/src/msg.cpp
	../../exosite-tests/src/http_parser.c
	../../exosite-tests/src/lib-async-tests.cpp
	../../exosite-tests/include/msg.h
	../../exosite-tests/include/http_parser.h
	../../exosite-pal/unit_tests_async/include/exosite_pal_async.h
	../../exosite-lib/include/exosite.h
)

IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${test_sync_srcs} PROPERTIES LANGUAGE CXX)
SET_SOURCE_FILES_PROPERTIES( ${test_async_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_executable(testrunner_sync ${test_sync_srcs} )
set_target_properties (testrunner_sync PROPERTIES COMPILE_DEFINITIONS "TESTING")
target_link_libraries(testrunner_sync exosite-sync-pal exosite-sync-lib gtest)

add_executable(testrunner_async ${test_async_srcs} )
set_target_properties (testrunner_async PROPERTIES COMPILE_DEFINITIONS "TESTING")
target_link_libraries(testrunner_async exosite-async-pal exosite-async-lib gtest)

