add_library(isa STATIC
    src/sf_tensor.c
    src/sf_opcodes.c
)
add_library(SionFlow::isa ALIAS isa)

target_include_directories(isa PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(isa PUBLIC

    base

)
