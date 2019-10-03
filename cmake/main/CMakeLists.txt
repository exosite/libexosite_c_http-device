include_directories(
    ../../exosite-lib/include/
    ../../exosite-pal/unit_tests_sync/include/
    ../../exosite-pal/unit_tests_async/include/
    )

set(main_srcs ../../main/main.c)

add_executable(main ${main_srcs})
target_link_libraries(main exosite-sync-pal exosite-sync-lib)

