set(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../sf-spec")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    GENERATOR "Ninja"
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME sf-spec CONFIG_PATH lib/cmake/sf-spec)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# Copy license/copyright
if(EXISTS "${SOURCE_PATH}/LICENSE")
    file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/sf-spec" RENAME copyright)
else()
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/sf-spec/copyright" "Copyright (c) SionFlow")
endif()