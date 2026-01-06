add_library(sf_base STATIC
    src/sf_memory.c
    src/sf_thread_pool.c
    src/sf_utils.c
    src/sf_log.c
    src/sf_platform.c
    src/sf_buffer.c
    src/sf_json.c
    src/sf_shape.c
)
add_library(SionFlow::base ALIAS sf_base)

target_include_directories(sf_base 
    PUBLIC include
    PRIVATE src
)

find_package(Threads REQUIRED)
target_link_libraries(sf_base PUBLIC Threads::Threads)

if(UNIX)
    # Math library is often needed for low-level ops if used in base
    target_link_libraries(sf_base PRIVATE m)
endif()
