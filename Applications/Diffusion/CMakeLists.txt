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
  org.commontk.configadmin
  org.commontk.eventadmin
  org.blueberry.osgi
  org.blueberry.compat
  org.blueberry.core.runtime
  org.blueberry.core.expressions
  org.blueberry.solstice.common
  org.blueberry.core.commands
  org.blueberry.ui
  org.blueberry.ui.qt
  org.blueberry.ui.qt.log
  org.blueberry.ui.qt.help
  org.mitk.core.services
  org.mitk.gui.common
  org.mitk.planarfigure
  org.mitk.core.ext
  org.mitk.diffusionimaging
  org.mitk.gui.qt.application
  org.mitk.gui.qt.ext
  org.mitk.gui.qt.diffusionimagingapp
  org.mitk.gui.qt.common
  org.mitk.gui.qt.stdmultiwidgeteditor
  org.mitk.gui.qt.common.legacy
  org.mitk.gui.qt.datamanager
  org.mitk.gui.qt.measurementtoolbox
  org.mitk.gui.qt.segmentation
  org.mitk.gui.qt.volumevisualization
  org.mitk.gui.qt.diffusionimaging
  org.mitk.gui.qt.imagenavigator
  org.mitk.gui.qt.moviemaker
  org.mitk.gui.qt.basicimageprocessing
  org.mitk.gui.qt.registration
  org.mitk.gui.qt.properties
  org.mitk.gui.qt.viewnavigator
  org.mitk.gui.qt.cmdlinemodules
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

FunctionCreateBlueBerryApplication(
  NAME ${DIFFUSIONAPP_NAME}
  DESCRIPTION "MITK Diffusion"
  PLUGINS ${_plugins}
  EXCLUDE_PLUGINS ${_exclude_plugins}
  ${_app_options}
)
mitk_use_modules(TARGET ${DIFFUSIONAPP_NAME} MODULES qtsingleapplication)

# Add meta dependencies (e.g. on auto-load modules from depending modules)
if(ALL_META_DEPENDENCIES)
  add_dependencies(${DIFFUSIONAPP_NAME} ${ALL_META_DEPENDENCIES})
endif()

# Add a build time dependency to legacy BlueBerry bundles.
if(MITK_MODULES_ENABLED_PLUGINS)
  add_dependencies(${DIFFUSIONAPP_NAME} ${MITK_MODULES_ENABLED_PLUGINS})
endif()
