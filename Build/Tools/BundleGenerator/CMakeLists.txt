SET(BUNDLE_DEFAULTS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/BundleDefaults.cmake")

ADD_SUBDIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/../../../Utilities/openCherry/Build"
                 "${CMAKE_CURRENT_BINARY_DIR}")
#INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../../Utilities/openCherry/Build/CMakeLists.txt)