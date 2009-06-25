PROJECT(MITKBundleGenerator)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(BUNDLE_DEFAULTS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/BundleDefaults.cmake")

ADD_SUBDIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/../../../openCherry/Build/BundleGenerator"
                 "${CMAKE_CURRENT_BINARY_DIR}/Generated")
#INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../../Utilities/openCherry/Build/CMakeLists.txt)