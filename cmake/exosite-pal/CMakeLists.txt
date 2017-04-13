# Need to compile files as CPP in order to get VS to "comply" with C99 stds

include_directories(
  ../../exosite-pal/unit_tests_sync/include/
  ../../exosite-pal/unit_tests_async/include/
)

set(pal_sync_srcs
   ../../exosite-pal/unit_tests_sync/src/exosite_pal.c
   ../../exosite-pal/unit_tests_sync/include/exosite_pal.h
   ../../exosite-pal/unit_tests_sync/include/exosite_pal_private.h
)

set(pal_async_srcs
   ../../exosite-pal/unit_tests_async/src/exosite_pal_async.c
   ../../exosite-pal/unit_tests_async/include/exosite_pal_async.h
)

IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${pal_sync_srcs} PROPERTIES LANGUAGE CXX)
SET_SOURCE_FILES_PROPERTIES( ${pal_async_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_library(exosite-sync-pal STATIC ${pal_sync_srcs})
add_library(exosite-async-pal STATIC ${pal_async_srcs})

