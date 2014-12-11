message(STATUS "Configuring MITK Diffusion Release Build")

# Disable all apps but MITK Diffusion
set(MITK_BUILD_ALL_APPS OFF CACHE BOOL "Build all MITK applications" FORCE)
set(MITK_BUILD_APP_CoreApp OFF CACHE BOOL "Build the MITK CoreApp" FORCE)
set(MITK_BUILD_APP_Workbench OFF CACHE BOOL "Build the MITK Workbench" FORCE)
set(MITK_BUILD_APP_Diffusion ON CACHE BOOL "Build MITK Diffusion" FORCE)

# Activate Diffusion Mini Apps
set(BUILD_DiffusionMiniApps ON CACHE BOOL "Build MITK Diffusion MiniApps" FORCE)

# Build neither all plugins nor examples
set(MITK_BUILD_ALL_PLUGINS OFF CACHE BOOL "Build all MITK plugins" FORCE)
set(MITK_BUILD_EXAMPLES OFF CACHE BOOL "Build the MITK examples" FORCE)

# Activate in-application help generation
set(MITK_DOXYGEN_GENERATE_QCH_FILES ON CACHE BOOL "Use doxygen to generate Qt compressed help files for MITK docs" FORCE)
set(BLUEBERRY_USE_QT_HELP ON CACHE BOOL "Enable support for integrating bundle documentation into Qt Help" FORCE)

# Disable console window
set(MITK_SHOW_CONSOLE_WINDOW OFF CACHE BOOL "Use this to enable or disable the console window when starting MITK GUI Applications" FORCE)

# Activate required plugins
# should be identical to the list in /Applications/Diffusion/CMakeLists.txt
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
)
