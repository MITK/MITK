if (${CMAKE_SOURCE_DIR} EQUAL ${PROJECT_SOURCE_DIR})
  cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
endif()

project(MitkPluginGenerator)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 5)
set(VERSION_PATCH 0)

set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(standalone_build 1)
else()
  set(standalone_build 0)
endif()

#-----------------------------------------------------------------------------
# Prerequisites
#-----------------------------------------------------------------------------

find_package(Qt5Core REQUIRED)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/PluginGeneratorConfig.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/PluginGeneratorConfig.h" @ONLY)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

#-----------------------------------------------------------------------------
# Executable
#-----------------------------------------------------------------------------

set(src_files
  PluginGenerator.cpp
  ctkCommandLineParser.cpp
)

qt5_wrap_cpp(src_files ctkCommandLineParser.h OPTIONS -DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
qt5_add_resources(src_files plugin_template.qrc project_template.qrc)

set(exec_target ${PROJECT_NAME})
add_executable(${exec_target} ${src_files})
set_property(TARGET ${exec_target} PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Plugins/PluginGenerator")

target_link_libraries(${exec_target} Qt5::Core)

if(NOT standalone_build)
  # subproject support
  add_dependencies(MITK-CoreUI ${exec_target})
endif()

#-----------------------------------------------------------------------------
# Win32 Convenience
#-----------------------------------------------------------------------------

if(WIN32 AND NOT standalone_build)
  file(TO_NATIVE_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}" native_runtime_dir)
  add_custom_target(NewPlugin start "MITK PluginGenerator" /D "${native_runtime_dir}" cmd /K ${exec_target}.exe -h
                    DEPENDS ${exec_target})
  set_property(TARGET NewPlugin PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Plugins/PluginGenerator")
endif()

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

if(NOT standalone_build)
  # Test the plugin generator
  include(mitkTestPluginGenerator)
endif()

#-----------------------------------------------------------------------------
# Packaging support
#-----------------------------------------------------------------------------

if(standalone_build)
  include(SetupPackaging.cmake)
endif()
