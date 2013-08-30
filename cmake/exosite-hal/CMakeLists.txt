# Need to compile files as CPP in order to get VS to "comply" with C99 stds

include_directories(
  ../../exosite-hal/unit_tests/include/
)


set(hal_srcs
   ../../exosite-hal/unit_tests/src/exosite_hal.c
   ../../exosite-hal/unit_tests/include/exosite_hal.h
)

IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${hal_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_library(exosite-hal STATIC ${hal_srcs})