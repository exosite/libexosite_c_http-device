include_directories(
  ../../exosite-lib/include/
)

set(lib_srcs
    ../../exosite-lib/src/exosite.c
    ../../exosite-lib/include/exosite.h
)

IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${lib_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_library(exosite-lib STATIC ${lib_srcs})

target_link_libraries(exosite-lib exosite-hal)