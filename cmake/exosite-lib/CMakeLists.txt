include_directories(
  ../../exosite-lib/include/
  ../../exosite-pal/unit_tests_sync/include/
  ../../exosite-pal/unit_tests_async/include/
  )

set(lib_sync_srcs
    ../../exosite-lib/src/exosite.c
    ../../exosite-lib/include/exosite.h
)

set(lib_async_srcs
    ../../exosite-lib/src/exosite_async.c
    ../../exosite-lib/include/exosite_async.h
)

IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${lib_sync_srcs} PROPERTIES LANGUAGE CXX)
SET_SOURCE_FILES_PROPERTIES( ${lib_async_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_library(exosite-sync-lib STATIC ${lib_sync_srcs})
add_library(exosite-async-lib STATIC ${lib_async_srcs})

target_link_libraries(exosite-sync-lib)
