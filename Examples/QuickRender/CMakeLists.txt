project(QuickRender)

set(PROJECT_NAME QuickRender)
set(APP_NAME MitkQuickRender)

# Plug-ins listed below will not be
#  - added as a build-time dependency to the executable
#  - listed in the provisioning file for the executable
#  - installed if they are external plug-ins

set(_exclude_plugins
  org.mitk.diffusionimaging
  org.mitk.simulation
  org.mitk.gui.qt.cmdlinemodules
  org.mitk.gui.qt.diffusionimagingapp
  org.mitk.gui.qt.datamanager
  org.mitk.gui.qt.datamanagerlight
  org.mitk.gui.qt.properties
  org.mitk.gui.qt.basicimageprocessing
  org.mitk.gui.qt.dicom
  org.mitk.gui.qt.diffusionimaging
  org.mitk.gui.qt.dosevisualization
  org.mitk.gui.qt.geometrytools
  org.mitk.gui.qt.igtexamples
  org.mitk.gui.qt.igttracking
  org.mitk.gui.qt.igtlplugin
  org.mitk.gui.qt.imagecropper
  org.mitk.gui.qt.imagenavigator
  org.mitk.gui.qt.viewnavigator
  org.mitk.gui.qt.materialeditor
  org.mitk.gui.qt.measurementtoolbox
  org.mitk.gui.qt.moviemaker
  org.mitk.gui.qt.pointsetinteraction
  org.mitk.gui.qt.pointsetinteractionmultispectrum
  org.mitk.gui.qt.python
  org.mitk.gui.qt.registration
  org.mitk.gui.qt.remeshing
  org.mitk.gui.qt.segmentation
  org.mitk.gui.qt.simulation
  org.mitk.gui.qt.aicpregistration
  org.mitk.gui.qt.toftutorial
  org.mitk.gui.qt.tofutil
  org.mitk.gui.qt.tubegraph
  org.mitk.gui.qt.ugvisualization
  org.mitk.gui.qt.ultrasound
  org.mitk.gui.qt.volumevisualization
  org.mitk.gui.qt.eventrecorder
  org.mitk.gui.qt.xnat
  org.mitk.gui.qt.spectrocamrecorder
  org.mitk.gui.qt.classificationsegmentation
)

mitkFunctionCreateBlueBerryApplication(
  NAME ${APP_NAME}
  DESCRIPTION "MITK - ${APP_NAME} Application"
  EXCLUDE_PLUGINS ${_exclude_plugins}
  NO_INSTALL
)

mitk_use_modules(TARGET ${APP_NAME}
  MODULES MitkAppUtil MitkQmlItems
  PACKAGES Qt5|Core+Quick
)
