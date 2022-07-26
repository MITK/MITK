message(STATUS "Configuring MITK Navigation Modules Build")

set(MITK_CONFIG_PACKAGES
  ACVD
  Qt5
  BLUEBERRY
)

# Enable open cv and open igt link, which is a necessary configuration
set(MITK_USE_OpenCV ON CACHE BOOL "MITK Use OpenCV Library" FORCE)
set(MITK_USE_OpenIGTLink ON CACHE BOOL "MITK Use OpenIGTLink Library" FORCE)

# Enable default plugins and the navigation modules
set(MITK_CONFIG_PLUGINS
  org.mitk.gui.qt.datamanager
  org.mitk.gui.qt.stdmultiwidgeteditor
  org.mitk.gui.qt.dicombrowser
  org.mitk.gui.qt.imagenavigator
  org.mitk.gui.qt.measurementtoolbox
  org.mitk.gui.qt.properties
  org.mitk.gui.qt.segmentation
  org.mitk.gui.qt.volumevisualization
  org.mitk.planarfigure
  org.mitk.gui.qt.moviemaker
  org.mitk.gui.qt.pointsetinteraction
  org.mitk.gui.qt.registration
  org.mitk.gui.qt.remeshing
  org.mitk.gui.qt.viewnavigator
  org.mitk.gui.qt.imagecropper
  org.mitk.gui.qt.igttracking
  org.mitk.gui.qt.openigtlink
  org.mitk.gui.qt.ultrasound
  org.mitk.gui.qt.tofutil
)

