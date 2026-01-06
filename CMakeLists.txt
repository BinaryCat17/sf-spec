cmake_minimum_required(VERSION 3.25)
project(sf-spec LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- ISA Generation ---
find_package(Python3 REQUIRED)

set(ISA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/isa/isa.json")
set(GEN_INC "${CMAKE_CURRENT_SOURCE_DIR}/generated/include/sionflow/isa/sf_ops_db.inc")
set(ISA_GEN_PY "${CMAKE_CURRENT_SOURCE_DIR}/tools/isa_gen.py")

add_custom_command(
    OUTPUT "${GEN_INC}"
    COMMAND Python3::Interpreter "${ISA_GEN_PY}" "${ISA_JSON}" "${GEN_INC}"
    DEPENDS "${ISA_JSON}" "${ISA_GEN_PY}"
    COMMENT "Generating SionFlow ISA headers from ${ISA_JSON}"
    VERBATIM
)

# Dummy target to force generation before other libraries
add_custom_target(isa_gen DEPENDS "${GEN_INC}")

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

# Install headers
install(DIRECTORY base/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY isa/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY generated/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

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