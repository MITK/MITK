#!
#! Create a BlueBerry application.
#!
#! \brief This function will create a BlueBerry application together with all
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
#! \param LIBRARY_DIRS A list of directories to pass through to MITK_INSTALL_TARGETS
#! \param SHOW_CONSOLE (option) Show the console output window (on Windows).
#! \param NO_PROVISIONING (option) Do not create provisioning files.
#! \param NO_INSTALL (option) Do not install this executable
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

macro_parse_arguments(_APP "NAME;DESCRIPTION;SOURCES;PLUGINS;EXCLUDE_PLUGINS;LINK_LIBRARIES;LIBRARY_DIRS" "SHOW_CONSOLE;NO_PROVISIONING;NO_INSTALL" ${ARGN})

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
  list(APPEND _APP_PLUGINS ${_plugin_deps})
endif()

#------------------------------------------------------------------------
# Prerequesites
#------------------------------------------------------------------------

find_package(MITK REQUIRED)
find_package(Poco REQUIRED)

# -----------------------------------------------------------------------
# Set up include and link dirs for the executable
# -----------------------------------------------------------------------

include_directories(
  ${org_blueberry_osgi_INCLUDE_DIRS}
)

# -----------------------------------------------------------------------
# Add executable icon (Windows)
# -----------------------------------------------------------------------
set(WINDOWS_ICON_RESOURCE_FILE "")
if(WIN32)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/icons/${_APP_NAME}.rc")
    set(WINDOWS_ICON_RESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/icons/${_APP_NAME}.rc")
  endif()
endif()

# -----------------------------------------------------------------------
# Create the executable and link libraries
# -----------------------------------------------------------------------

set(_app_compile_flags )
if(WIN32)
  set(_app_compile_flags "${_app_compile_flags} -DPOCO_NO_UNWINDOWS -DWIN32_LEAN_AND_MEAN")
endif()

# Work-around for linking GDCM libraries, until GDCM provides proper
# target exports.
foreach(dir ${MODULES_PACKAGE_DEPENDS_DIRS})
  if(EXISTS "${dir}/MITK_GDCM_Config.cmake")
    include("${dir}/MITK_GDCM_Config.cmake")
    break()
  endif()
endforeach()
if(ALL_LIBRARY_DIRS)
  list(REMOVE_DUPLICATES ALL_LIBRARY_DIRS)
  link_directories(${ALL_LIBRARY_DIRS})
endif()

if(_APP_SHOW_CONSOLE)
  add_executable(${_APP_NAME} MACOSX_BUNDLE ${_APP_SOURCES} ${WINDOWS_ICON_RESOURCE_FILE})
else()
  add_executable(${_APP_NAME} MACOSX_BUNDLE WIN32 ${_APP_SOURCES} ${WINDOWS_ICON_RESOURCE_FILE})
endif()
mitk_use_modules(TARGET ${_APP_NAME} MODULES mbilog PACKAGES Poco Qt4|QtCore Qt5|Core)

set_target_properties(${_APP_NAME} PROPERTIES
                      COMPILE_FLAGS "${_app_compile_flags}")

target_link_libraries(${_APP_NAME} org_blueberry_osgi ${_APP_LINK_LIBRARIES})
if(WIN32)
  target_link_libraries(${_APP_NAME} ${QT_QTMAIN_LIBRARY})
endif()

# -----------------------------------------------------------------------
# Add executable icon and bundle name (Mac)
# -----------------------------------------------------------------------
if(APPLE)
  if( _APP_DESCRIPTION)
    set_target_properties(${_APP_NAME} PROPERTIES MACOSX_BUNDLE_NAME "${_APP_DESCRIPTION}")
  else()
    set_target_properties(${_APP_NAME} PROPERTIES MACOSX_BUNDLE_NAME "${_APP_NAME}")
  endif()
  set(icon_name "icon.icns")
  set(icon_full_path "${CMAKE_CURRENT_SOURCE_DIR}/icons/${icon_name}")
  if(EXISTS "${icon_full_path}")
    set_target_properties(${_APP_NAME} PROPERTIES MACOSX_BUNDLE_ICON_FILE "${icon_name}")
    file(COPY ${icon_full_path} DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_APP_NAME}.app/Contents/Resources/")
    INSTALL (FILES ${icon_full_path} DESTINATION "${_APP_NAME}.app/Contents/Resources/")
  endif()
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
  set(template_name "start${_APP_NAME}.bat.in")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${template_name}")
    foreach(BUILD_TYPE debug release)
      mitkFunctionCreateWindowsBatchScript(${template_name}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/start${_APP_NAME}_${BUILD_TYPE}.bat
        ${BUILD_TYPE})
    endforeach()
  endif()
endif(WIN32)

# -----------------------------------------------------------------------
# Install support
# -----------------------------------------------------------------------

if(NOT _APP_NO_INSTALL)

  # This installs all third-party CTK plug-ins
  FunctionInstallThirdPartyCTKPlugins(${_APP_PLUGINS} EXCLUDE ${_APP_EXCLUDE_PLUGINS})

  if(COMMAND BlueBerryApplicationInstallHook)
    set(_real_app_plugins ${_APP_PLUGINS})
    if(_APP_EXCLUDE_PLUGINS)
      list(REMOVE_ITEM _real_app_plugins ${_APP_EXCLUDE_PLUGINS})
    endif()
    BlueBerryApplicationInstallHook(APP_NAME ${_APP_NAME} PLUGINS ${_real_app_plugins})
  endif()

  # Install the executable
  MITK_INSTALL_TARGETS(EXECUTABLES ${_APP_NAME} LIBRARY_DIRS ${_APP_LIBRARY_DIRS} GLOB_PLUGINS )

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

endif()

endfunction()
