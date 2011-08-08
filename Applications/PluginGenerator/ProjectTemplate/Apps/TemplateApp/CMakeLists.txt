#-----------------------------------------------------------------------------
# Prerequesites
#-----------------------------------------------------------------------------

find_package(Qt REQUIRED)

#-----------------------------------------------------------------------------
# Include and linker directories
#-----------------------------------------------------------------------------

include(${QT_USE_FILE})

include_directories(
  ${org_blueberry_osgi_INCLUDE_DIRS}
  ${Poco_INCLUDE_DIRS}
  ${mbilog_INCLUDE_DIRS}
  ${ALL_INCLUDE_DIRECTORIES}
)

#-----------------------------------------------------------------------------
# Executable
#-----------------------------------------------------------------------------

if(${MY_PROJECT_NAME}_SHOW_CONSOLE_WINDOW)
  add_executable(${MY_APP_NAME} MACOSX_BUNDLE ${MY_APP_NAME}.cpp)
else()
  add_executable(${MY_APP_NAME} MACOSX_BUNDLE WIN32 ${MY_APP_NAME}.cpp)
endif()

target_link_libraries(${MY_APP_NAME} org_blueberry_osgi ${ALL_LIBRARIES})
if(WIN32)
  target_link_libraries(${MY_APP_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTMAIN_LIBRARY})
endif()

# Make sure all enabled plug-ins are up to date when building the application
ctkFunctionExtractPluginTargets("${PROJECT_PLUGINS}" ON PROJECT_ENABLED_PLUGINS)
if(PROJECT_ENABLED_PLUGINS)
  add_dependencies(${MY_APP_NAME} ${PROJECT_ENABLED_PLUGINS})
endif()

#-----------------------------------------------------------------------------
# Additional files needed for the executable
#-----------------------------------------------------------------------------

# Create a provisioning file, listing all enabled plug-ins
set(PROJECT_PROVISIONING_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${MY_APP_NAME}.provisioning")
FunctionCreateProvisioningFile(
  FILE ${PROJECT_PROVISIONING_FILE}
  INCLUDE "${MITK_PLUGIN_PROVISIONING_FILE}"
  PLUGIN_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins"
  PLUGINS ${PROJECT_PLUGINS}
  )

# Create a .ini file for initial parameters
configure_file(${MY_APP_NAME}.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${MY_APP_NAME}.ini)

# Create batch files for Windows platforms
if(WIN32)
  foreach(BUILD_TYPE debug release)
    mitkFunctionCreateWindowsBatchScript(start${MY_APP_NAME}.bat.in
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/start${MY_APP_NAME}_${BUILD_TYPE}.bat
      ${BUILD_TYPE})
  endforeach()
endif()

#-----------------------------------------------------------------------------
# Install support
#-----------------------------------------------------------------------------

# This installs all enabled CTK plug-ins
FunctionInstallThirdPartyCTKPlugins(${CTK_PLUGIN_LIBRARIES})

# Install the executable and its provisioning file
MITK_INSTALL_TARGETS(EXECUTABLES ${MY_APP_NAME} GLOB_PLUGINS)
mitkFunctionInstallProvisioningFiles(${MITK_PROVISIONING_FILES}
                                     ${PROJECT_PROVISIONING_FILE})

# On Linux, create a shell script to start a relocatable application
if(UNIX AND NOT APPLE)
  install(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledApp.sh" DESTINATION "." RENAME ${MY_APP_NAME}.sh)
endif()

# Tell cpack the executables that you want in the start menu as links
set(MITK_CPACK_PACKAGE_EXECUTABLES "${MITK_CPACK_PACKAGE_EXECUTABLES}" "${MY_APP_NAME};MITK - ${MY_APP_NAME} Application" CACHE INTERNAL "Collecting windows shortcuts to executables")

