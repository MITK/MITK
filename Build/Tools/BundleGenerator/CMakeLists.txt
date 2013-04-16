project(MITKBundleGenerator)

cmake_minimum_required(VERSION 2.6)

set(BUNDLE_DEFAULTS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/BundleDefaults.cmake")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../../BlueBerry/Build/BundleGenerator"
                 "${CMAKE_CURRENT_BINARY_DIR}/Generated")
#include(${CMAKE_CURRENT_SOURCE_DIR}/../../../Utilities/BlueBerry/Build/CMakeLists.txt)