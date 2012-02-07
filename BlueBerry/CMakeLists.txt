PROJECT(BlueBerry)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake/")

INCLUDE(MacroParseArguments)
INCLUDE(MacroConvertSchema)
INCLUDE(MacroOrganizeSources)
INCLUDE(berryPluginHelpers)
INCLUDE(MacroCollectPlugins)
INCLUDE(MacroParseManifest)
INCLUDE(MacroCreatePlugin)
INCLUDE(MacroCreateCTKPlugin)
INCLUDE(MacroCreateQtHelp)
INCLUDE(MacroInstallPlugin)
INCLUDE(MacroInstallCTKPlugin)
INCLUDE(FunctionCreateProvisioningFile)

IF(MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4275 /wd4251 /wd4503")
ENDIF()

IF (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
ENDIF ()

FIND_PACKAGE(mbilog REQUIRED)
INCLUDE_DIRECTORIES(${mbilog_INCLUDE_DIRS})

OPTION(BLUEBERRY_USE_QT "Use the Qt GUI toolkit" OFF)

IF(NOT DESIRED_QT_VERSION)
  SET(DESIRED_QT_VERSION 4 CACHE STRING "Desired Qt version" FORCE)
  MARK_AS_ADVANCED(DESIRED_QT_VERSION)
ENDIF()

IF(BLUEBERRY_USE_QT AND NOT DESIRED_QT_VERSION EQUAL 4)
  MESSAGE("Attention: Qt4 GUI libraries are required to build the BlueBerry Qt plug-ins.")
ENDIF()

IF(BLUEBERRY_USE_QT AND DESIRED_QT_VERSION EQUAL 4)
  SET(BUILD_QT_PLUGINS 1)
  FIND_PACKAGE(Qt4 4.6.2 REQUIRED)

  IF(QT_QMAKE_CHANGED)
    SET(QT_HELPGENERATOR_EXECUTABLE NOTFOUND)
    SET(QT_COLLECTIONGENERATOR_EXECUTABLE NOTFOUND)
    SET(QT_ASSISTANT_EXECUTABLE NOTFOUND)
    SET(QT_XMLPATTERNS_EXECUTABLE NOTFOUND)
  ENDIF()

  FIND_PROGRAM(QT_HELPGENERATOR_EXECUTABLE
    NAMES qhelpgenerator qhelpgenerator-qt4 qhelpgenerator4
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
  )

  FIND_PROGRAM(QT_COLLECTIONGENERATOR_EXECUTABLE
    NAMES qcollectiongenerator qcollectiongenerator-qt4 qcollectiongenerator4
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
  )

  FIND_PROGRAM(QT_ASSISTANT_EXECUTABLE
    NAMES assistant-qt4 assistant4 assistant
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
  )

  FIND_PROGRAM(QT_XMLPATTERNS_EXECUTABLE
    NAMES xmlpatterns
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
  )

  OPTION(BLUEBERRY_USE_QT_HELP "Enable support for integrating bundle documentation into Qt Help" ON)

  MARK_AS_ADVANCED(BLUEBERRY_USE_QT_HELP
                   QT_HELPGENERATOR_EXECUTABLE
                   QT_COLLECTIONGENERATOR_EXECUTABLE
                   QT_ASSISTANT_EXECUTABLE
                   QT_XMLPATTERNS_EXECUTABLE)

  SET(_doxygen_too_old 1)
  IF(BLUEBERRY_USE_QT_HELP)
    FIND_PACKAGE(Doxygen)
    IF(DOXYGEN_FOUND)
      EXECUTE_PROCESS(COMMAND ${DOXYGEN_EXECUTABLE} --version
                      OUTPUT_VARIABLE _doxygen_version)
      IF(${_doxygen_version} VERSION_GREATER 1.6.0 OR
         ${_doxygen_version} VERSION_EQUAL 1.6.0)
        SET(_doxygen_too_old 0)
      ENDIF()
    ENDIF()
  ELSE(BLUEBERRY_USE_QT_HELP)
  
  CONFIGURE_FILE(../Documentation/pregenerated/MITKBlankPage.qch
                 ${MITK_BINARY_DIR}/bin/ExtBundles/org.mitk.gui.qt.extapplication/resources/MITKBlankPage.qch
                 COPYONLY)

  CONFIGURE_FILE(../Documentation/pregenerated/MitkExtQtHelpCollection.qhc
                 ${MITK_BINARY_DIR}/bin/ExtBundles/org.mitk.gui.qt.extapplication/resources/MitkExtQtHelpCollection.qhc
                 COPYONLY)
  
  ENDIF(BLUEBERRY_USE_QT_HELP)

  IF (BLUEBERRY_USE_QT_HELP AND _doxygen_too_old)
    MESSAGE("Doxygen was not found or is too old. Version 1.6.0 or later is needed if BLUEBERRY_USE_QT_HELP is ON")
    SET(BLUEBERRY_USE_QT_HELP OFF CACHE BOOL "Enable support for integrating bundle documentation into Qt Help" FORCE)
  ENDIF()

  IF(BLUEBERRY_USE_QT_HELP AND NOT QT_HELPGENERATOR_EXECUTABLE)
    MESSAGE("You have enabled Qt Help support, but QT_HELPGENERATOR_EXECUTABLE is empty")
    SET(BLUEBERRY_USE_QT_HELP OFF CACHE BOOL "Enable support for integrating bundle documentation into Qt Help" FORCE)
  ENDIF()

  IF(BLUEBERRY_USE_QT_HELP AND NOT QT_XMLPATTERNS_EXECUTABLE)
    MESSAGE("You have enabled Qt Help support, but QT_XMLPATTERNS_EXECUTABLE is empty")
    SET(BLUEBERRY_USE_QT_HELP OFF CACHE BOOL "Enable support for integrating bundle documentation into Qt Help" FORCE)
  ENDIF()

  INCLUDE(${QT_USE_FILE})
ELSE()
  FIND_PACKAGE(Qt4 4.6.2 COMPONENTS QtCore REQUIRED)
  INCLUDE(${QT_USE_FILE})
ENDIF()

# =========  CTK specific CMake stuff ============
CMAKE_POLICY(SET CMP0012 NEW)

FIND_PACKAGE(CTK REQUIRED)

# Extract all library names starting with org_blueberry_
MACRO(GetMyTargetLibraries all_target_libraries varname)
  SET(re_ctkplugin "^org_blueberry_[a-zA-Z0-9_]+$")
  SET(_tmp_list)
  LIST(APPEND _tmp_list ${all_target_libraries})
  ctkMacroListFilter(_tmp_list re_ctkplugin OUTPUT_VARIABLE ${varname})
ENDMACRO()
# ================================================

OPTION(BLUEBERRY_BUILD_ALL_PLUGINS "Build all BlueBerry plugins (overriding selection)" OFF)
MARK_AS_ADVANCED(BLUEBERRY_BUILD_ALL_PLUGINS)

IF(BLUEBERRY_BUILD_ALL_PLUGINS)
  SET(BLUEBERRY_BUILD_ALL_PLUGINS_OPTION "FORCE_BUILD_ALL")
ENDIF()

OPTION(BLUEBERRY_STATIC "Build all plugins as static libraries" OFF)
MARK_AS_ADVANCED(BLUEBERRY_STATIC)

OPTION(BLUEBERRY_DEBUG_SMARTPOINTER "Enable code for debugging smart pointers" OFF)
MARK_AS_ADVANCED(BLUEBERRY_DEBUG_SMARTPOINTER)

FIND_PACKAGE(Poco REQUIRED)
FIND_PACKAGE(Ant)
FIND_PACKAGE(Eclipse)

SET(BLUEBERRY_SOURCE_DIR ${BlueBerry_SOURCE_DIR})
SET(BLUEBERRY_BINARY_DIR ${BlueBerry_BINARY_DIR})

SET(BLUEBERRY_PLUGINS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Bundles)
SET(BLUEBERRY_PLUGINS_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Bundles)

SET(OSGI_APP solstice)
SET(OSGI_UI_APP solstice_ui)

# Force should be removed after everybody has configured their old binary tree
SET(BLUEBERRY_PLUGINS_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BlueBerry CACHE PATH "Directory where to build the BlueBerry Bundles" FORCE)
MARK_AS_ADVANCED(BLUEBERRY_PLUGINS_OUTPUT_DIR)

# Clear the cache variables
SET(BLUEBERRY_PLUGIN_SOURCE_DIRS "" CACHE INTERNAL "List of base plugin source directories" FORCE)
SET(BLUEBERRY_PLUGIN_BINARY_DIRS "" CACHE INTERNAL "List of base plugin binary directories" FORCE)

IF (Eclipse_DIR)
  SET(BLUEBERRY_DOC_TOOLS_DIR "${Eclipse_DIR}" CACHE PATH "Directory containing additional tools needed for generating the documentation")
ELSE ()
  SET(BLUEBERRY_DOC_TOOLS_DIR "" CACHE PATH "Directory containing additional tools needed for generating the documentation")
ENDIF ()

SET(BLUEBERRY_DEBUG_POSTFIX d)

# Testing options

OPTION(BLUEBERRY_BUILD_TESTING "Build the BlueBerry tests." ${BUILD_TESTING})

IF(WIN32)
  SET(_gui_testing_default "ON")
ELSE()
  SET(_gui_testing_default "OFF")
ENDIF()

OPTION(BLUEBERRY_ENABLE_GUI_TESTING "Enable the BlueBerry GUI tests" ${_gui_testing_default})
MARK_AS_ADVANCED(BLUEBERRY_ENABLE_GUI_TESTING)

IF(BLUEBERRY_BUILD_TESTING)
  ENABLE_TESTING()
ENDIF()


# Add CTK plugins

SET(_ctk_plugins
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
  Bundles/org.blueberry.ui.qt.log:OFF
  Bundles/org.blueberry.ui.qt.objectinspector:OFF
)

SET(_ctk_test_plugins )

SET(_ctk_plugins_include_dirs
  ${Poco_INCLUDE_DIRS}
)

SET(_ctk_plugins_link_dirs
  ${Poco_LIBRARY_DIR}
)

INCLUDE_DIRECTORIES(${_ctk_plugins_include_dirs})
LINK_DIRECTORIES(${_ctk_plugins_link_dirs})

IF(BLUEBERRY_BUILD_TESTING)
  INCLUDE(berryTestingHelpers)

  SET(BLUEBERRY_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_APP}")
  GET_TARGET_PROPERTY(_is_macosx_bundle ${OSGI_APP} MACOSX_BUNDLE)
  IF(APPLE AND _is_macosx_bundle) 
    SET(BLUEBERRY_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_APP}.app/Contents/MacOS/${OSGI_APP}")
  ENDIF()

  SET(_ctk_testinfrastructure_plugins
    Bundles/org.blueberry.test:ON
    Bundles/org.blueberry.uitest:ON
  )

  SET(_ctk_test_plugins
#    Testing/org.blueberry.core.runtime.tests:ON
#    Testing/org.blueberry.osgi.tests:ON
  )
  
  IF(BLUEBERRY_ENABLE_GUI_TESTING)
#    LIST(APPEND _ctk_test_plugins Testing/org.blueberry.ui.tests:ON)

    SET(BLUEBERRY_UI_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_UI_APP}")
    GET_TARGET_PROPERTY(_is_macosx_bundle ${OSGI_UI_APP} MACOSX_BUNDLE)
    IF(APPLE AND _is_macosx_bundle)
      SET(BLUEBERRY_UI_TEST_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OSGI_UI_APP}.app/Contents/MacOS/${OSGI_UI_APP}")
    ENDIF()
  ENDIF()
ENDIF()

SET(BLUEBERRY_TESTING_PROVISIONING_FILE "${BlueBerry_BINARY_DIR}/BlueBerryTesting.provisioning")

ADD_CUSTOM_TARGET(BlueBerry)

ctkMacroSetupPlugins(${_ctk_plugins} ${_ctk_testinfrastructure_plugins} ${_ctk_test_plugins}
                     BUILD_OPTION_PREFIX BLUEBERRY_BUILD_
                     BUILD_ALL ${BLUEBERRY_BUILD_ALL_PLUGINS}
                     COMPACT_OPTIONS)

SET(BLUEBERRY_PROVISIONING_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BlueBerry.provisioning")
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

SET(BB_PLUGIN_USE_FILE "${BlueBerry_BINARY_DIR}/BlueBerryPluginUseFile.cmake")
IF(${PROJECT_NAME}_PLUGIN_LIBRARIES)
  ctkFunctionGeneratePluginUseFile(${BB_PLUGIN_USE_FILE})
ELSE()
  FILE(REMOVE ${BB_PLUGIN_USE_FILE})
  SET(BB_PLUGIN_USE_FILE )
ENDIF()

# CTK Plugin Exports
SET(BB_PLUGIN_EXPORTS_FILE "${CMAKE_CURRENT_BINARY_DIR}/BlueBerryPluginExports.cmake")
GetMyTargetLibraries("${${PROJECT_NAME}_PLUGIN_LIBRARIES}" my_plugin_targets)
SET(additional_export_targets mbilog PocoFoundation PocoUtil PocoXML)
IF(BLUEBERRY_BUILD_TESTING)
  LIST(APPEND additional_export_targets CppUnit)
ENDIF()
export(TARGETS ${my_plugin_targets} ${additional_export_targets}
       FILE ${BB_PLUGIN_EXPORTS_FILE})


ADD_SUBDIRECTORY(Documentation)

CONFIGURE_FILE(BlueBerryConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/BlueBerryConfig.cmake @ONLY)


