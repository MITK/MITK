#-----------------------------------------------------------------------------
# Prerequesites
#-----------------------------------------------------------------------------

find_package(Qt REQUIRED)

#-----------------------------------------------------------------------------
# Include and linker directories
#-----------------------------------------------------------------------------

INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(
  ${org_blueberry_osgi_INCLUDE_DIRS}
  ${Poco_INCLUDE_DIRS}
  ${mbilog_INCLUDE_DIRS}
  ${ALL_INCLUDE_DIRECTORIES}
)

LINK_DIRECTORIES(${MITK_LIBRARY_DIRS})

#-----------------------------------------------------------------------------
# Executable
#-----------------------------------------------------------------------------

set(DIFFUSIONAPP_NAME mitkDiffusion)

IF(MITK_SHOW_CONSOLE_WINDOW)
  ADD_EXECUTABLE(${DIFFUSIONAPP_NAME} MACOSX_BUNDLE ${DIFFUSIONAPP_NAME}.cpp)
ELSE()
  ADD_EXECUTABLE(${DIFFUSIONAPP_NAME} MACOSX_BUNDLE WIN32 ${DIFFUSIONAPP_NAME}.cpp)
ENDIF()

TARGET_LINK_LIBRARIES(${DIFFUSIONAPP_NAME} org_blueberry_osgi ${ALL_LIBRARIES})
IF(WIN32)
  TARGET_LINK_LIBRARIES(${DIFFUSIONAPP_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTMAIN_LIBRARY})
ENDIF()

# Make sure all enabled plug-ins are up to date when building the application
ctkFunctionExtractPluginTargets("${MITKDIFFUSION_PLUGINS}" ON _plugin_deps)

IF(_plugin_deps)
  # Make sure all enabled plug-ins are up to date
  add_dependencies(${DIFFUSIONAPP_NAME} ${_plugin_deps})
ENDIF()

#-----------------------------------------------------------------------------
# Additional files needed for the executable
#-----------------------------------------------------------------------------

# Create a provisioning file, listing all enabled plug-ins

set(DIFFUSIONIMAGINGAPP_PROVISIONING_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${DIFFUSIONAPP_NAME}.provisioning")
FunctionCreateProvisioningFile(
  FILE ${DIFFUSIONIMAGINGAPP_PROVISIONING_FILE}
  INCLUDE "${MITK_PLUGIN_PROVISIONING_FILE}"
)

# Create a .ini file for initial parameters
CONFIGURE_FILE(${DIFFUSIONAPP_NAME}.ini
               ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${DIFFUSIONAPP_NAME}.ini)

# Create batch files for Windows platforms
IF(WIN32)
  FOREACH(DIFFUSIONIMAGINGAPP_BUILD_TYPE debug release)
    mitkFunctionCreateWindowsBatchScript(start${DIFFUSIONAPP_NAME}.bat.in
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/start${DIFFUSIONAPP_NAME}_${DIFFUSIONIMAGINGAPP_BUILD_TYPE}.bat
      ${DIFFUSIONIMAGINGAPP_BUILD_TYPE})
  ENDFOREACH()
ENDIF()


#-----------------------------------------------------------------------------
# Source Files
#-----------------------------------------------------------------------------

SET(CPP_FILES
 mitkDiffusion.cpp
 #mitkDiffusionImagingSafeQApplication.cpp
 QDiffusionImagingSplashScreen.cpp
)

QT4_WRAP_CPP(CPP_FILES QDiffusionImagingSplashScreen.h)
QT4_ADD_RESOURCES(CPP_FILES splashscreen.qrc)

#-----------------------------------------------------------------------------
# Bundle Activation
#-----------------------------------------------------------------------------

SET(_mitk_dependencies
org_mitk_diffusionimaging
org_mitk_gui_qt_diffusionimagingapp
org_mitk_gui_qt_common_legacy
org_mitk_gui_qt_ext
org_mitk_gui_qt_datamanager
org_mitk_gui_qt_measurementtoolbox
org_mitk_gui_qt_segmentation
org_mitk_gui_qt_volumevisualization
org_mitk_gui_qt_diffusionimaging
org_mitk_gui_qt_imagenavigator
org_mitk_gui_qt_moviemaker
org_mitk_planarfigure
)

# check for missing mitk dependencies
FOREACH(_mitk_diffusion_dep ${_mitk_dependencies})
  IF(NOT TARGET ${_mitk_diffusion_dep})
    MESSAGE(FATAL_ERROR "missing dependency: ${_mitk_diffusion_dep}")
  ENDIF()
ENDFOREACH()

#-----------------------------------------------------------------------------
# Install support
#-----------------------------------------------------------------------------

# This installs all enabled CTK plug-ins
FunctionInstallThirdPartyCTKPlugins(${CTK_PLUGIN_LIBRARIES})

# Install the executable and its provisioning file
MITK_INSTALL_TARGETS(EXECUTABLES ${DIFFUSIONAPP_NAME} GLOB_PLUGINS)
mitkFunctionInstallProvisioningFiles(${MITK_PROVISIONING_FILES}
										${DIFFUSIONIMAGINGAPP_PROVISIONING_FILE})

# On Linux, create a shell script to start a relocatable application
IF(UNIX AND NOT APPLE)
  INSTALL(PROGRAMS "${MITK_SOURCE_DIR}/CMake/RunInstalledApp.sh" DESTINATION "." RENAME ${DIFFUSIONAPP_NAME}.sh)
ENDIF()

# tell cpack the executables that you also want mitkDiffusion in the start menu as links
# mitkDiffusionSET(MITK_CPACK_PACKAGE_EXECUTABLES "${MITK_CPACK_PACKAGE_EXECUTABLES}" "${DIFFUSIONAPP_NAME}; MITK Diffusion" CACHE INTERNAL "Collecting windows shortcuts to executables")
