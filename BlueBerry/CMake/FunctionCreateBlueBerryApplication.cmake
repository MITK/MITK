#!
#! Create a BlueBerry application.
#!
#! \brif This function will create a BlueBerry application together with all
#! necessary provisioning and configuration data and install support.
#!
#! \param NAME (required) The name of the executable.
#! \param DESCRIPTION (optional) A human-readable description of your application.
#!        The usage depends on the CPack generator (on Windows, this is a descriptive
#!        text for the created shortcuts).
#! \param SOURCES (optional) A list of source files to compile into your executable. Defaults
#!        to <name>.cpp.
#! \param PLUGINS (optional) A list of required plug-ins. Defaults to all known plug-ins.
#! \param EXCLUDE_PLUGINS (optional) A list of plug-ins which should not be used. Mainly
#!        useful if PLUGINS was not used.
#! \param LINK_LIBRARIES A list of libraries to be linked with the executable.
#! \param SHOW_CONSOLE (option) Show the console output window (on Windows).
#! \param NO_PROVISIONING (option) Do not create provisioning files.
#!
#! Assuming that there exists a file called <code>MyApp.cpp</code>, an example call looks like:
#! \code
#! FunctionCreateBlueBerryApplication(
#!   NAME MyApp
#!   DESCRIPTION "MyApp - New ways to explore medical data"
#!   EXCLUDE_PLUGINS org.mitk.gui.qt.extapplication
#!   SHOW_CONSOLE
#! )
#! \endcode
#!
function(FunctionCreateBlueBerryApplication)

macro_parse_arguments(_APP "NAME;DESCRIPTION;SOURCES;PLUGINS;EXCLUDE_PLUGINS;LINK_LIBRARIES" "SHOW_CONSOLE;NO_PROVISIONING" ${ARGN})

if(NOT _APP_NAME)
  message(FATAL_ERROR "NAME argument cannot be empty.")
endif()

if(NOT _APP_SOURCES)
  set(_APP_SOURCES ${_APP_NAME}.cpp)
endif()

if(NOT _APP_PLUGINS)
  ctkFunctionGetAllPluginTargets(_APP_PLUGINS)
else()
  set(_plugins ${_APP_PLUGINS})
  set(_APP_PLUGINS)
  foreach(_plugin ${_plugins})
    string(REPLACE "." "_" _plugin_target ${_plugin})
    list(APPEND _APP_PLUGINS ${_plugin_target})
  endforeach()
  
  # get all plug-in dependencies
  ctkFunctionGetPluginDependencies(_plugin_deps PLUGINS ${_APP_PLUGINS} ALL)
  # add the dependencies to the list of application plug-ins
  list (APPEND _APP_PLUGINS ${_plugin_deps})
endif()

#------------------------------------------------------------------------
# Prerequesites
#------------------------------------------------------------------------

find_package(MITK REQUIRED)

# -----------------------------------------------------------------------
# Set up include and link dirs for the executable
# -----------------------------------------------------------------------

include(${QT_USE_FILE})

include_directories(
  ${org_blueberry_osgi_INCLUDE_DIRS}
  ${Poco_INCLUDE_DIRS}
  ${mbilog_INCLUDE_DIRS}
)

link_directories(${MITK_LINK_DIRECTORIES})

# -----------------------------------------------------------------------
# Create the executable and link libraries
# -----------------------------------------------------------------------

if(_APP_SHOW_CONSOLE)
  add_executable(${_APP_NAME} MACOSX_BUNDLE ${_APP_SOURCES})
else()
  add_executable(${_APP_NAME} MACOSX_BUNDLE WIN32 ${_APP_SOURCES})
endif()

target_link_libraries(${_APP_NAME} org_blueberry_osgi ${_APP_LINK_LIBRARIES})
if(WIN32)
  target_link_libraries(${_APP_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTMAIN_LIBRARY})
endif()

# -----------------------------------------------------------------------
# Set build time dependencies
# -----------------------------------------------------------------------

# This ensures that all enabled plug-ins are up-to-date when the
# executable is build.
if(_APP_PLUGINS)
  ctkMacroGetAllProjectTargetLibraries("${_APP_PLUGINS}" _project_plugins)
  if(_APP_EXCLUDE_PLUGINS)
    set(_exclude_targets)
    foreach(_exclude_plugin ${_APP_EXCLUDE_PLUGINS})
      string(REPLACE "." "_" _exclude_target ${_exclude_plugin})
      list(APPEND _exclude_targets ${_exclude_target})
    endforeach()
    list(REMOVE_ITEM _project_plugins ${_exclude_targets})
  endif()
  if(_project_plugins)
    add_dependencies(${_APP_NAME} ${_project_plugins})
  endif()
endif()

# -----------------------------------------------------------------------
# Additional files needed for the executable
# -----------------------------------------------------------------------

if(NOT _APP_NO_PROVISIONING)

  # Create a provisioning file, listing all plug-ins
  set(_prov_file "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_APP_NAME}.provisioning")
  FunctionCreateProvisioningFile(FILE ${_prov_file}
                                 PLUGINS ${_APP_PLUGINS}
                                 EXCLUDE_PLUGINS ${_APP_EXCLUDE_PLUGINS}
                                )

endif()

# Create a .ini file for initial parameters
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_APP_NAME}.ini")
  configure_file(${_APP_NAME}.ini
                 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_APP_NAME}.ini)
endif()

# Create batch files for Windows platforms
if(WIN32)
  foreach(BUILD_TYPE debug release)
    mitkFunctionCreateWindowsBatchScript(start${_APP_NAME}.bat.in
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/start${_APP_NAME}_${BUILD_TYPE}.bat
      ${BUILD_TYPE})
  endforeach()
endif(WIN32)

# -----------------------------------------------------------------------
# Install support
# -----------------------------------------------------------------------

# This installs all third-party CTK plug-ins
FunctionInstallThirdPartyCTKPlugins(${_APP_PLUGINS} EXCLUDE ${_APP_EXCLUDE_PLUGINS})

# Install the executable
MITK_INSTALL_TARGETS(EXECUTABLES ${_APP_NAME} GLOB_PLUGINS )

if(NOT _APP_NO_PROVISIONING)
  # Install the provisioning file
  mitkFunctionInstallProvisioningFiles(${_prov_file})
endif()

# On Linux, create a shell script to start a relocatable application
if(UNIX AND NOT APPLE)
  install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledApp.sh" DESTINATION "." RENAME ${_APP_NAME}.sh)
endif()

# Tell cpack the executables that you want in the start menu as links
set(MITK_CPACK_PACKAGE_EXECUTABLES ${MITK_CPACK_PACKAGE_EXECUTABLES} "${_APP_NAME};${_APP_DESCRIPTION}" CACHE INTERNAL "Collecting windows shortcuts to executables")

endfunction()
