project(BlueBerry)
cmake_minimum_required(VERSION 2.8.4)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake/")

include(MacroParseArguments)
include(MacroConvertSchema)
include(MacroOrganizeSources)
include(MacroCreateCTKPlugin)
include(MacroCreateQtHelp)
include(MacroInstallCTKPlugin)
include(FunctionCreateProvisioningFile)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4275 /wd4251 /wd4503")
endif()

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

find_package(mbilog REQUIRED)
include_directories(${mbilog_INCLUDE_DIRS})


find_package(Qt4 4.6.2 REQUIRED)

if(QT_QMAKE_CHANGED)
  set(QT_HELPGENERATOR_EXECUTABLE NOTFOUND)
  set(QT_COLLECTIONGENERATOR_EXECUTABLE NOTFOUND)
  set(QT_ASSISTANT_EXECUTABLE NOTFOUND)
  set(QT_XMLPATTERNS_EXECUTABLE NOTFOUND)
endif()

find_program(QT_HELPGENERATOR_EXECUTABLE
  NAMES qhelpgenerator qhelpgenerator-qt4 qhelpgenerator4
  PATHS ${QT_BINARY_DIR}
  NO_DEFAULT_PATH
)

find_program(QT_COLLECTIONGENERATOR_EXECUTABLE
  NAMES qcollectiongenerator qcollectiongenerator-qt4 qcollectiongenerator4
  PATHS ${QT_BINARY_DIR}
  NO_DEFAULT_PATH
)

find_program(QT_ASSISTANT_EXECUTABLE
  NAMES assistant-qt4 assistant4 assistant
  PATHS ${QT_BINARY_DIR}
  NO_DEFAULT_PATH
)

find_program(QT_XMLPATTERNS_EXECUTABLE
  NAMES xmlpatterns
  PATHS ${QT_BINARY_DIR}
  NO_DEFAULT_PATH
)

option(BLUEBERRY_USE_QT_HELP "Enable support for integrating bundle documentation into Qt Help" ON)

mark_as_advanced(BLUEBERRY_USE_QT_HELP
                 QT_HELPGENERATOR_EXECUTABLE
                 QT_COLLECTIONGENERATOR_EXECUTABLE
                 QT_ASSISTANT_EXECUTABLE
                 QT_XMLPATTERNS_EXECUTABLE)

set(_doxygen_too_old 1)
if(BLUEBERRY_USE_QT_HELP)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    execute_process(COMMAND ${DOXYGEN_EXECUTABLE} --version
                    OUTPUT_VARIABLE _doxygen_version)
    if(${_doxygen_version} VERSION_GREATER 1.6.0 OR
       ${_doxygen_version} VERSION_EQUAL 1.6.0)
      set(_doxygen_too_old 0)
    endif()
  endif()

  if(_doxygen_too_old)
    message("Doxygen was not found or is too old. Version 1.6.0 or later is needed if BLUEBERRY_USE_QT_HELP is ON")
    set(BLUEBERRY_USE_QT_HELP OFF CACHE BOOL "Enable support for integrating bundle documentation into Qt Help" FORCE)
  endif()

  if(NOT QT_HELPGENERATOR_EXECUTABLE)
    message("You have enabled Qt Help support, but QT_HELPGENERATOR_EXECUTABLE is empty")
    set(BLUEBERRY_USE_QT_HELP OFF CACHE BOOL "Enable support for integrating bundle documentation into Qt Help" FORCE)
  endif()

  if(NOT QT_XMLPATTERNS_EXECUTABLE)
    message("You have enabled Qt Help support, but QT_XMLPATTERNS_EXECUTABLE is empty")
    set(BLUEBERRY_USE_QT_HELP OFF CACHE BOOL "Enable support for integrating bundle documentation into Qt Help" FORCE)
  endif()
endif(BLUEBERRY_USE_QT_HELP)

include(${QT_USE_FILE})

# =========  CTK specific CMake stuff ============
cmake_policy(SET CMP0012 NEW)

find_package(CTK REQUIRED)

# Extract all library names starting with org_blueberry_
macro(GetMyTargetLibraries all_target_libraries varname)
  set(re_ctkplugin "^org_blueberry_[a-zA-Z0-9_]+$")
  set(_tmp_list)
  list(APPEND _tmp_list ${all_target_libraries})
  ctkMacroListFilter(_tmp_list re_ctkplugin OUTPUT_VARIABLE ${varname})
endmacro()
# ================================================

option(BLUEBERRY_BUILD_ALL_PLUGINS "Build all BlueBerry plugins (overriding selection)" OFF)
mark_as_advanced(BLUEBERRY_BUILD_ALL_PLUGINS)

if(BLUEBERRY_BUILD_ALL_PLUGINS)
  set(BLUEBERRY_BUILD_ALL_PLUGINS_OPTION "FORCE_BUILD_ALL")
endif()

option(BLUEBERRY_STATIC "Build all plugins as static libraries" OFF)
mark_as_advanced(BLUEBERRY_STATIC)

option(BLUEBERRY_DEBUG_SMARTPOINTER "Enable code for debugging smart pointers" OFF)
mark_as_advanced(BLUEBERRY_DEBUG_SMARTPOINTER)

find_package(Poco REQUIRED)
find_package(Ant)
find_package(Eclipse)

set(BLUEBERRY_SOURCE_DIR ${BlueBerry_SOURCE_DIR})
set(BLUEBERRY_BINARY_DIR ${BlueBerry_BINARY_DIR})

set(BLUEBERRY_PLUGINS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Bundles)
set(BLUEBERRY_PLUGINS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Bundles)

set(OSGI_APP solstice)
set(OSGI_UI_APP solstice_ui)

if(Eclipse_DIR)
  set(BLUEBERRY_DOC_TOOLS_DIR "${Eclipse_DIR}" CACHE PATH "Directory containing additional tools needed for generating the documentation")
else()
  set(BLUEBERRY_DOC_TOOLS_DIR "" CACHE PATH "Directory containing additional tools needed for generating the documentation")
endif()

set(BLUEBERRY_DEBUG_POSTFIX d)

# Testing options

if(DEFINED BLUEBERRY_BUILD_TESTING)
  option(BLUEBERRY_BUILD_TESTING "Build the BlueBerry tests." ${BLUEBERRY_BUILD_TESTING})
else()
  option(BLUEBERRY_BUILD_TESTING "Build the BlueBerry tests." ${BUILD_TESTING})
endif()

if(WIN32)
  set(_gui_testing_default "ON")
else()
  set(_gui_testing_default "OFF")
endif()

option(BLUEBERRY_ENABLE_GUI_TESTING "Enable the BlueBerry GUI tests" ${_gui_testing_default})
mark_as_advanced(BLUEBERRY_ENABLE_GUI_TESTING)

if(BLUEBERRY_BUILD_TESTING)
  enable_testing()
endif()


# Add CTK plugins

set(_ctk_plugins
  Bundles/org.blueberry.osgi:ON
  Bundles/org.blueberry.compat:OFF
  Bundles/org.blueberry.core.runtime:OFF
  Bundles/org.blueberry.core.expressions:OFF
  Bundles/org.blueberry.solstice.common:OFF
  Bundles/org.blueberry.core.commands:OFF
  Bundles/org.blueberry.core.jobs:OFF
  Bundles/org.blueberry.ui:OFF
  Bundles/org.blueberry.ui.qt:OFF
  Bundles/org.blueberry.ui.qt.help:OFF
  Bundles/org.blueberry.ui.qt.log:ON
  Bundles/org.blueberry.ui.qt.objectinspector:OFF
)

set(_ctk_test_plugins )

set(_ctk_plugins_include_dirs
  ${Poco_INCLUDE_DIRS}
)

set(_ctk_plugins_link_dirs
  ${Poco_LIBRARY_DIR}
)

include_directories(${_ctk_plugins_include_dirs})
link_directories(${_ctk_plugins_link_dirs})

if(BLUEBERRY_BUILD_TESTING)
  include(berryTestingHelpers)

  set(BLUEBERRY_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_APP}")
  get_target_property(_is_macosx_bundle ${OSGI_APP} MACOSX_BUNDLE)
  if(APPLE AND _is_macosx_bundle) 
    set(BLUEBERRY_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_APP}.app/Contents/MacOS/${OSGI_APP}")
  endif()

  set(_ctk_testinfrastructure_plugins
    Bundles/org.blueberry.test:ON
    Bundles/org.blueberry.uitest:ON
  )

  set(_ctk_test_plugins
#    Testing/org.blueberry.core.runtime.tests:ON
#    Testing/org.blueberry.osgi.tests:ON
  )
  
  if(BLUEBERRY_ENABLE_GUI_TESTING)
#    list(APPEND _ctk_test_plugins Testing/org.blueberry.ui.tests:ON)

    set(BLUEBERRY_UI_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_UI_APP}")
    get_target_property(_is_macosx_bundle ${OSGI_UI_APP} MACOSX_BUNDLE)
    if(APPLE AND _is_macosx_bundle)
      set(BLUEBERRY_UI_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_UI_APP}.app/Contents/MacOS/${OSGI_UI_APP}")
    endif()
  endif()
endif()

set(BLUEBERRY_TESTING_PROVISIONING_FILE "${BlueBerry_BINARY_DIR}/BlueBerryTesting.provisioning")

add_custom_target(BlueBerry)

ctkMacroSetupPlugins(${_ctk_plugins} ${_ctk_testinfrastructure_plugins} ${_ctk_test_plugins}
                     BUILD_OPTION_PREFIX BLUEBERRY_BUILD_
                     BUILD_ALL ${BLUEBERRY_BUILD_ALL_PLUGINS}
                     COMPACT_OPTIONS)

set(BLUEBERRY_PROVISIONING_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BlueBerry.provisioning")
FunctionCreateProvisioningFile(
  FILE ${BLUEBERRY_PROVISIONING_FILE}
  PLUGINS ${_ctk_plugins}
)

FunctionCreateProvisioningFile(
  FILE ${BLUEBERRY_TESTING_PROVISIONING_FILE}
  INCLUDE ${BLUEBERRY_PROVISIONING_FILE}
  PLUGINS ${_ctk_testinfrastructure_plugins} ${_ctk_test_plugins}
)

if(${CMAKE_PROJECT_NAME}_PLUGIN_LIBRARIES)
  add_dependencies(BlueBerry ${${CMAKE_PROJECT_NAME}_PLUGIN_LIBRARIES})
endif()
set_property(TARGET ${${CMAKE_PROJECT_NAME}_PLUGIN_LIBRARIES} PROPERTY LABELS BlueBerry)

set(BB_PLUGIN_USE_FILE "${BlueBerry_BINARY_DIR}/BlueBerryPluginUseFile.cmake")
if(${PROJECT_NAME}_PLUGIN_LIBRARIES)
  ctkFunctionGeneratePluginUseFile(${BB_PLUGIN_USE_FILE})
else()
  file(REMOVE ${BB_PLUGIN_USE_FILE})
  set(BB_PLUGIN_USE_FILE )
endif()

# CTK Plugin Exports
set(BB_PLUGIN_EXPORTS_FILE "${CMAKE_CURRENT_BINARY_DIR}/BlueBerryPluginExports.cmake")
GetMyTargetLibraries("${${PROJECT_NAME}_PLUGIN_LIBRARIES}" my_plugin_targets)
set(additional_export_targets mbilog PocoFoundation PocoUtil PocoXML)
if(BLUEBERRY_BUILD_TESTING)
  list(APPEND additional_export_targets CppUnit)
endif()
export(TARGETS ${my_plugin_targets} ${additional_export_targets}
       FILE ${BB_PLUGIN_EXPORTS_FILE})


add_subdirectory(Documentation)

set(BLUEBERRY_QTPLUGIN_PATH )
if(CTK_QTDESIGNERPLUGINS_DIR AND EXISTS ${CTK_QTDESIGNERPLUGINS_DIR})
  set(BLUEBERRY_QTPLUGIN_PATH "${CTK_QTDESIGNERPLUGINS_DIR}")
endif()

configure_file(BlueBerryConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/BlueBerryConfig.cmake @ONLY)

