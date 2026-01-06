cmake_minimum_required(VERSION 3.25)
project(sf-spec LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Modules
add_subdirectory(base)
add_subdirectory(isa)

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