cmake_minimum_required(VERSION 3.25)
project(sf-spec LANGUAGES C)

enable_testing()

set(SF_SPEC_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Path to sf-spec root")

# --- Configuration ---
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# --- Code Generation ---
find_package(Python3 REQUIRED COMPONENTS Interpreter)

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c "import jinja2"
    RESULT_VARIABLE JINJA2_NOT_FOUND
    OUTPUT_QUIET ERROR_QUIET
)

if(JINJA2_NOT_FOUND)
    message(STATUS "Python3 executable found at: ${Python3_EXECUTABLE}")
    message(FATAL_ERROR "Python3 Jinja2 package is required for code generation. Install it for this specific interpreter or ensure you are using the correct Python.")
endif()

set(SF_ISA_METADATA "${CMAKE_CURRENT_SOURCE_DIR}/tools/metadata/isa.json")
set(SF_ISA_GEN_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tools/isa_gen.py")
set(SF_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(SF_ISA_GENERATED_FILES
    "${SF_GENERATED_DIR}/src/sf_opcodes.c"
    "${SF_GENERATED_DIR}/include/sionflow/isa/sf_opcodes.h"
    "${SF_GENERATED_DIR}/include/sionflow/isa/sf_isa_constants.h"
)

add_custom_command(
    OUTPUT ${SF_ISA_GENERATED_FILES}
    COMMAND Python3::Interpreter "${SF_ISA_GEN_SCRIPT}"
        --isa "${SF_ISA_METADATA}"
        --render 
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_opcodes.c.j2:${SF_GENERATED_DIR}/src/sf_opcodes.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_opcodes.h.j2:${SF_GENERATED_DIR}/include/sionflow/isa/sf_opcodes.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_isa_constants.h.j2:${SF_GENERATED_DIR}/include/sionflow/isa/sf_isa_constants.h"
    DEPENDS 
        "${SF_ISA_METADATA}"
        "${SF_ISA_GEN_SCRIPT}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_opcodes.c.j2"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_opcodes.h.j2"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates/sf_isa_constants.h.j2"
    COMMENT "Generating ISA source files from metadata..."
    VERBATIM
)

add_custom_target(sf_spec_gen DEPENDS ${SF_ISA_GENERATED_FILES})

# --- Subdirectories ---
add_subdirectory(base)
add_subdirectory(isa)

# --- Export & Install ---
set(SF_ISA_METADATA "${CMAKE_CURRENT_SOURCE_DIR}/tools/metadata/isa.json" CACHE INTERNAL "Path to ISA metadata")
set(SF_MANIFEST_METADATA "${CMAKE_CURRENT_SOURCE_DIR}/tools/metadata/manifest.json" CACHE INTERNAL "Path to Manifest metadata")
set(SF_ISA_GEN_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tools/isa_gen.py" CACHE INTERNAL "Path to ISA generator script")
set(SF_ISA_TEMPLATES "${CMAKE_CURRENT_SOURCE_DIR}/tools/templates" CACHE INTERNAL "Path to ISA templates")

install(TARGETS base isa
    EXPORT SionFlowSpecTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY base/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY isa/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ${SF_GENERATED_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

export(EXPORT SionFlowSpecTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/SionFlowSpecTargets.cmake"
    NAMESPACE SionFlow::
)

configure_package_config_file(
    cmake/sf-spec-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sf-spec-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sf-spec
)

install(EXPORT SionFlowSpecTargets
    FILE SionFlowSpecTargets.cmake
    NAMESPACE SionFlow::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sf-spec
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/sf-spec-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sf-spec
)

# --- Workspace Mode (Delayed) ---
# We add sibling modules AFTER our targets are defined so they can see them.
set(SF_WORKSPACE OFF CACHE BOOL "Build all SionFlow modules as a single project")

if(SF_WORKSPACE)
    message(STATUS "SionFlow: Workspace Mode Enabled")
    set(SF_WORKSPACE_MODE ON CACHE BOOL "" FORCE)
    
    # sf-compiler
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../sf-compiler/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../sf-compiler" "${CMAKE_CURRENT_BINARY_DIR}/sf-compiler")
    endif()

    # sf-backend-cpu
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../sf-backend-cpu/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../sf-backend-cpu" "${CMAKE_CURRENT_BINARY_DIR}/sf-backend-cpu")
    endif()

    # sf-runtime
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../sf-runtime/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../sf-runtime" "${CMAKE_CURRENT_BINARY_DIR}/sf-runtime")
    endif()

    # sf-samples
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../sf-samples/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../sf-samples" "${CMAKE_CURRENT_BINARY_DIR}/sf-samples")
    endif()
endif()
