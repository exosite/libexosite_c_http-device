# Need to compile files as CPP in order to get VS to "comply" with C99 stds

include_directories(
  ../../exosite-pal/include/
  ../../exosite-pal/unit_tests/include/
)


set(pal_srcs
   ../../exosite-pal/unit_tests/src/exosite_pal.c
   ../../exosite-pal/include/exosite_pal.h
   ../../exosite-pal/unit_tests/include/exosite_pal_private.h
)

IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${pal_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_library(exosite-pal STATIC ${pal_srcs})
