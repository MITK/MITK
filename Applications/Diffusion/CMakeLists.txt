project(MitkDiffusion)

set(DIFFUSIONAPP_NAME MitkDiffusion)

set(_app_options)
if(MITK_SHOW_CONSOLE_WINDOW)
  list(APPEND _app_options SHOW_CONSOLE)
endif()

# Create a cache entry for the provisioning file which is used to export
# the file name in the MITKConfig.cmake file. This will keep external projects
# which rely on this file happy.
set(DIFFUSIONIMAGINGAPP_PROVISIONING_FILE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${DIFFUSIONAPP_NAME}.provisioning" CACHE INTERNAL "${DIFFUSIONAPP_NAME} provisioning file" FORCE)

# should be identical to the list in /CMake/mitkBuildConfigurationmitkDiffusion.cmake
# remember to set plugins which should be automatically toggled in target_libraries.cmake
set(_plugins
  org.mitk.planarfigure
  org.mitk.gui.qt.diffusionimagingapp
  org.mitk.gui.qt.measurementtoolbox
  org.mitk.gui.qt.segmentation
  org.mitk.gui.qt.volumevisualization
  org.mitk.gui.qt.diffusionimaging
  org.mitk.gui.qt.diffusionimaging.fiberfox
  org.mitk.gui.qt.diffusionimaging.fiberprocessing
  org.mitk.gui.qt.diffusionimaging.ivim
  org.mitk.gui.qt.diffusionimaging.odfpeaks
  org.mitk.gui.qt.diffusionimaging.preprocessing
  org.mitk.gui.qt.diffusionimaging.reconstruction
  org.mitk.gui.qt.diffusionimaging.tractography
  org.mitk.gui.qt.diffusionimaging.connectomics
  org.mitk.gui.qt.imagenavigator
  org.mitk.gui.qt.moviemaker
  org.mitk.gui.qt.basicimageprocessing
  org.mitk.gui.qt.properties
  org.mitk.gui.qt.viewnavigator
)

# Plug-ins listed below will not be
#  - added as a build-time dependency to the executable
#  - listed in the provisioning file for the executable
#  - installed if they are external plug-ins

set(_exclude_plugins
  org.blueberry.test
  org.blueberry.uitest
  org.mitk.gui.qt.coreapplication
  org.mitk.gui.qt.extapplication
)

set(_src_files
  MitkDiffusion.cpp
)

qt5_add_resources(_src_files splashscreen.qrc)

mitkFunctionCreateBlueBerryApplication(
  NAME ${DIFFUSIONAPP_NAME}
  DESCRIPTION "MITK Diffusion"
  PLUGINS ${_plugins}
  EXCLUDE_PLUGINS ${_exclude_plugins}
  SOURCES ${_src_files}
  ${_app_options}
)

mitk_use_modules(TARGET ${DIFFUSIONAPP_NAME} MODULES MitkAppUtil)

# Add meta dependencies (e.g. on auto-load modules from depending modules)
if(TARGET ${CMAKE_PROJECT_NAME}-autoload)
  add_dependencies(${DIFFUSIONAPP_NAME} ${CMAKE_PROJECT_NAME}-autoload)
endif()

# Add a build time dependency to legacy BlueBerry bundles.
if(MITK_MODULES_ENABLED_PLUGINS)
  add_dependencies(${DIFFUSIONAPP_NAME} ${MITK_MODULES_ENABLED_PLUGINS})
endif()
