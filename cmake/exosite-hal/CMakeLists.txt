include_directories(
  ../../exosite-lib/include/
)

set(hal_srcs
   ../../exosite-lib/src/exosite_hal.c
   ../../exosite-lib/include/exosite_hal.h

)

# Need to compile files as CPP in order to get VS to "comply" with C99 stds
IF(MSVC)
SET_SOURCE_FILES_PROPERTIES( ${hal_srcs} PROPERTIES LANGUAGE CXX)
ENDIF()

add_library(exosite-hal STATIC ${hal_srcs})