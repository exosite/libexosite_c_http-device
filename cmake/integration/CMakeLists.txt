include_directories(
	../../exosite-lib/include/
	../../exosite-pal/integration_async/include/
	../../exosite-tests/integration/include/
)

set(integration_sync_srcs
	../../exosite-tests/unit/src/main.cpp
	../../exosite-pal/unit_tests_sync/include/exosite_pal_private.h
	../../exosite-lib/include/exosite.h
)

set(integration_async_srcs
	../../exosite-tests/integration/src/main.c
	../../exosite-pal/integration_async/src/exosite_pal_async.c
	../../exosite-pal/integration_async/include/exosite_pal_async.h
	../../exosite-lib/include/exosite.h
)

IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${integration_sync_srcs} PROPERTIES LANGUAGE CXX)
SET_SOURCE_FILES_PROPERTIES( ${integration_async_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

#add_executable(integration_sync ${integration_sync_srcs} )
#set_target_properties (integration_sync PROPERTIES COMPILE_DEFINITIONS "TESTING")
#target_link_libraries(integration_sync exosite-sync-pal integration-sync-lib)

add_executable(integration_async ${integration_async_srcs} )
set_target_properties (integration_async PROPERTIES COMPILE_DEFINITIONS "TESTING")
target_link_libraries(integration_async exosite-async-lib)

