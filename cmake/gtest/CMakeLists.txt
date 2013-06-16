include_directories(
  ../../gtest-1.6.0/include/gtest
  ../../gtest-1.6.0/include
  ../../gtest-1.6.0/src

)

set(gtest_srcs
    ../../gtest-1.6.0/src/gtest-all.cc
)

add_library(gtest STATIC ${gtest_srcs})
target_link_libraries(gtest ${CMAKE_THREAD_LIBS_INIT})
