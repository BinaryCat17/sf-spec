add_library(base STATIC
    src/sf_memory.c
    src/sf_thread_pool.c
    src/sf_utils.c
    src/sf_log.c
    src/sf_platform.c
    src/sf_buffer.c
    src/sf_json.c
    src/sf_shape.c
)
add_library(SionFlow::base ALIAS base)

target_include_directories(base 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE src
)

find_package(Threads REQUIRED)
target_link_libraries(base PUBLIC Threads::Threads)

if(UNIX)
    # Math library is often needed for low-level ops if used in base
    target_link_libraries(base PRIVATE m)
endif()
