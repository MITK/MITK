if (${CMAKE_SOURCE_DIR} EQUAL ${PROJECT_SOURCE_DIR})
  cmake_minimum_required(VERSION 2.8.9)
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

set(QT_DONT_USE_QTGUI 1)
set(QT_USE_QTNETWORK 1)
if(MITK_USE_Qt4)
  find_package(Qt 4.7 REQUIRED)
  include(${QT_USE_FILE})
else()
  find_package(Qt5Network REQUIRED)
endif()

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

if(MITK_USE_Qt4)
  qt4_wrap_cpp(src_files ctkCommandLineParser.h OPTIONS -DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
  qt4_add_resources(src_files plugin_template.qrc project_template.qrc)
else()
  qt5_wrap_cpp(src_files ctkCommandLineParser.h OPTIONS -DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
  qt5_add_resources(src_files plugin_template.qrc project_template.qrc)
endif()

set(exec_target ${PROJECT_NAME})
add_executable(${exec_target} ${src_files})

if(MITK_USE_Qt4)
  target_link_libraries(${exec_target} ${QT_LIBRARIES})
else()
  target_link_libraries(${exec_target} Qt5::Network)
endif()

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
