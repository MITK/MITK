message(STATUS "Configuring MITK Photoacoustics Setup Build")

# Enable open cv and open igt link, which is a necessary configuration
set(MITK_USE_OpenCV ON CACHE BOOL "MITK Use OpenCV Library" FORCE)
set(MITK_USE_OpenIGTLink ON CACHE BOOL "MITK Use OpenIGTLink Library" FORCE)

set(MITK_USE_OpenCL ON CACHE BOOL "MITK Use OpenCL Library" FORCE)

# Enable default plugins and the navigation modules
set(MITK_CONFIG_PLUGINS
  org.mitk.gui.qt.datamanager
  org.mitk.gui.qt.stdmultiwidgeteditor
  org.mitk.gui.qt.imagenavigator
  org.mitk.gui.qt.properties
  org.mitk.gui.qt.viewnavigator
  org.mitk.gui.qt.ultrasound
  org.mitk.gui.qt.photoacoustics.imageprocessing
  org.mitk.gui.qt.measurementtoolbox
  org.mitk.gui.qt.pointsetinteraction
)

