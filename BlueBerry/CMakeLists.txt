project(BlueBerry)
if (${CMAKE_SOURCE_DIR} EQUAL ${PROJECT_SOURCE_DIR})
  cmake_minimum_required(VERSION 2.8.9)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake/")

include(MacroParseArguments)
include(MacroConvertSchema)
include(MacroOrganizeSources)
include(MacroCreateCTKPlugin)
include(MacroCreateQtHelp)
include(MacroInstallCTKPlugin)
include(FunctionCreateProvisioningFile)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX /wd4250 /wd4275 /wd4251 /wd4503")
endif()

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()

find_package(mbilog REQUIRED)
include_directories(${mbilog_INCLUDE_DIRS})

if(MITK_USE_Qt4)
  find_package(Qt4 4.7 REQUIRED)
else()
  find_package(Qt5Concurrent ${MITK_QT5_MINIMUM_VERSION} REQUIRED)
  find_package(Qt5Help ${MITK_QT5_MINIMUM_VERSION} REQUIRED)
  find_package(Qt5Sql ${MITK_QT5_MINIMUM_VERSION} REQUIRED)
  find_package(Qt5WebKitWidgets ${MITK_QT5_MINIMUM_VERSION} REQUIRED)
  find_package(Qt5Widgets ${MITK_QT5_MINIMUM_VERSION} REQUIRED)
  find_package(Qt5Xml ${MITK_QT5_MINIMUM_VERSION} REQUIRED)
endif()

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

find_package(Doxygen)

option(BLUEBERRY_USE_QT_HELP "Enable support for integrating bundle documentation into Qt Help" ${DOXYGEN_FOUND})

mark_as_advanced(BLUEBERRY_USE_QT_HELP
                 QT_HELPGENERATOR_EXECUTABLE
                 QT_COLLECTIONGENERATOR_EXECUTABLE
                 QT_ASSISTANT_EXECUTABLE
                 QT_XMLPATTERNS_EXECUTABLE)

if(BLUEBERRY_USE_QT_HELP)
  set(_force_blueberry_use_qt_help_to_off 0)
  if(NOT DOXYGEN_FOUND)
    message("> Forcing BLUEBERRY_USE_QT_HELP to OFF because Doxygen was not found.")
    set(_force_blueberry_use_qt_help_to_off 1)
  endif()
  if(DOXYGEN_FOUND AND DOXYGEN_VERSION VERSION_LESS 1.8.7)
    message("> Forcing BLUEBERRY_USE_QT_HELP to OFF because Doxygen version 1.8.7 or newer not found.")
    set(_force_blueberry_use_qt_help_to_off 1)
  endif()

  if(NOT QT_HELPGENERATOR_EXECUTABLE)
    message("> Forcing BLUEBERRY_USE_QT_HELP to OFF because QT_HELPGENERATOR_EXECUTABLE is empty.")
    set(_force_blueberry_use_qt_help_to_off 1)
  endif()

  if(NOT QT_XMLPATTERNS_EXECUTABLE)
    message("You have enabled Qt Help support, but QT_XMLPATTERNS_EXECUTABLE is empty")
    set(_force_blueberry_use_qt_help_to_off 1)
  endif()

  if(_force_blueberry_use_qt_help_to_off)
    set(BLUEBERRY_USE_QT_HELP OFF CACHE BOOL "Enable support for integrating bundle documentation into Qt Help" FORCE)
  endif()
endif(BLUEBERRY_USE_QT_HELP)

if(MITK_USE_Qt4)
  include(${QT_USE_FILE})
endif()

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

include_directories(${_ctk_plugins_include_dirs})

if(BLUEBERRY_BUILD_TESTING)
  find_package(CppUnit REQUIRED)
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
set(additional_export_targets mbilog)
export(TARGETS ${my_plugin_targets} ${additional_export_targets}
       FILE ${BB_PLUGIN_EXPORTS_FILE})


add_subdirectory(Documentation)

set(BLUEBERRY_QTPLUGIN_PATH )
if(CTK_QTDESIGNERPLUGINS_DIR AND EXISTS ${CTK_QTDESIGNERPLUGINS_DIR})
  set(BLUEBERRY_QTPLUGIN_PATH "${CTK_QTDESIGNERPLUGINS_DIR}")
endif()

configure_file(BlueBerryConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/BlueBerryConfig.cmake @ONLY)

