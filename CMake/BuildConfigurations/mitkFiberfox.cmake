message(STATUS "Configuring MITK Fiberfox Release Build")

# Enable non-optional external dependencies
set(MITK_USE_Vigra ON CACHE BOOL "MITK Use Vigra Library" FORCE)
set(MITK_USE_HDF5 ON CACHE BOOL "MITK Use HDF5 Library" FORCE)

# Disable all apps but MITK Fiberfox
set(MITK_BUILD_ALL_APPS OFF CACHE BOOL "Build all MITK applications" FORCE)
set(MITK_BUILD_APP_CoreApp OFF CACHE BOOL "Build the MITK CoreApp" FORCE)
set(MITK_BUILD_APP_Workbench OFF CACHE BOOL "Build the MITK Workbench" FORCE)
set(MITK_BUILD_APP_Fiberfox ON CACHE BOOL "Build MITK Fiberfox" FORCE)

# Activate Diffusion Mini Apps
set(BUILD_DiffusionFiberfoxCmdApps ON CACHE BOOL "Build commandline tools for diffusion data simulation (Fiberfox)" FORCE)

# Build neither all plugins nor examples
set(MITK_BUILD_ALL_PLUGINS OFF CACHE BOOL "Build all MITK plugins" FORCE)
set(MITK_BUILD_EXAMPLES OFF CACHE BOOL "Build the MITK examples" FORCE)

# Activate in-application help generation
set(MITK_DOXYGEN_GENERATE_QCH_FILES ON CACHE BOOL "Use doxygen to generate Qt compressed help files for MITK docs" FORCE)
set(BLUEBERRY_USE_QT_HELP ON CACHE BOOL "Enable support for integrating bundle documentation into Qt Help" FORCE)

# Disable console window
set(MITK_SHOW_CONSOLE_WINDOW OFF CACHE BOOL "Use this to enable or disable the console window when starting MITK GUI Applications" FORCE)

# Activate required plugins
# should be identical to the list in /Applications/Fiberfox/CMakeLists.txt
set(_plugins
  org_blueberry_ui_qt
  org_blueberry_ui_qt_help
  org_mitk_planarfigure
  org_mitk_gui_qt_extapplication
  org_mitk_gui_qt_common_legacy
  org_mitk_gui_qt_ext
  org_mitk_gui_qt_datamanager
  org_mitk_gui_qt_segmentation
  org_mitk_gui_qt_volumevisualization
  org_mitk_gui_qt_diffusionimaging
  org_mitk_gui_qt_diffusionimaging_fiberfox
  org_mitk_gui_qt_diffusionimaging_fiberprocessing
  org_mitk_gui_qt_diffusionimaging_preprocessing
  org_mitk_gui_qt_diffusionimaging_reconstruction
  org_mitk_gui_qt_imagenavigator
  org_mitk_gui_qt_moviemaker
  org_mitk_gui_qt_measurementtoolbox
  org_mitk_gui_qt_viewnavigator
)
