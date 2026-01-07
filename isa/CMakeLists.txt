add_library(isa STATIC
    src/sf_tensor.c
    src/sf_exec_ctx.c
    "${SF_GENERATED_DIR}/src/sf_opcodes.c"
)
add_library(SionFlow::isa ALIAS isa)

add_dependencies(isa sf_spec_gen)

target_include_directories(isa PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${SF_GENERATED_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(isa PUBLIC

    base

)
