cmake_minimum_required(VERSION 3.25)
project(sf-spec LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- ISA Generation ---
find_package(Python3 REQUIRED)

set(ISA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/tools/metadata/isa.json")
set(COMPILER_SPEC "${CMAKE_CURRENT_SOURCE_DIR}/../sf-compiler/tools/metadata/compiler_spec.json")
set(ISA_GEN_PY "${CMAKE_CURRENT_SOURCE_DIR}/tools/isa_gen.py")
set(GEN_H "${CMAKE_CURRENT_SOURCE_DIR}/tools/generated/include/sionflow/isa/sf_opcodes.h")
set(GEN_CONST_H "${CMAKE_CURRENT_SOURCE_DIR}/tools/generated/include/sionflow/isa/sf_isa_constants.h")
set(GEN_C "${CMAKE_CURRENT_SOURCE_DIR}/tools/generated/src/sf_opcodes.c")

add_custom_command(
    OUTPUT "${GEN_H}" "${GEN_C}" "${GEN_CONST_H}"
    COMMAND Python3::Interpreter "${ISA_GEN_PY}" --isa "${ISA_JSON}" 
            --compiler "${COMPILER_SPEC}"
            --render "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_opcodes.h.j2:${GEN_H}"
                     "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_opcodes.c.j2:${GEN_C}"
                     "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_isa_constants.h.j2:${GEN_CONST_H}"
    DEPENDS "${ISA_JSON}" "${ISA_GEN_PY}" "${COMPILER_SPEC}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_opcodes.h.j2"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_opcodes.c.j2"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_isa_constants.h.j2"
    COMMENT "Generating SionFlow ISA (Headers & Metadata)"
    VERBATIM
)

# Dummy target to force generation before other libraries
add_custom_target(isa_gen DEPENDS "${GEN_H}" "${GEN_C}" "${GEN_CONST_H}")

# Modules
add_subdirectory(base)
add_subdirectory(isa)

add_dependencies(base isa_gen)
add_dependencies(isa isa_gen)

# --- Installation & Export ---

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install targets
install(TARGETS base isa
    EXPORT SionFlowSpecTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets for use from the build directory
export(EXPORT SionFlowSpecTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/SionFlowSpecTargets.cmake"
    NAMESPACE SionFlow::
)

# Install headers
install(DIRECTORY base/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY isa/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY tools/generated/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Generate and install the export file
install(EXPORT SionFlowSpecTargets
    FILE SionFlowSpecTargets.cmake
    NAMESPACE SionFlow::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sf-spec
)

# Create the config file
configure_package_config_file(
    cmake/sf-spec-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sf-spec-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sf-spec
)

# Install the config file
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/sf-spec-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sf-spec
)