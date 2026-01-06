add_library(sf_isa STATIC
    src/sf_tensor.c
    src/sf_opcodes.c
)
add_library(SionFlow::isa ALIAS sf_isa)

target_include_directories(sf_isa PUBLIC
    include
)

target_link_libraries(sf_isa PUBLIC
    SionFlow::base
)