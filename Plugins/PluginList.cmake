
# Plug-ins must be ordered according to their dependencies
if (MITK_USE_Qt4)

set(MITK_EXT_PLUGINS
  org.mitk.core.services:ON
  org.mitk.gui.common:ON
  org.mitk.planarfigure:ON
  org.mitk.core.ext:OFF
  org.mitk.core.jobs:OFF
  org.mitk.diffusionimaging:OFF
  org.mitk.simulation:OFF
  org.mitk.gui.qt.application:ON
  org.mitk.gui.qt.coreapplication:OFF
  org.mitk.gui.qt.ext:OFF
  org.mitk.gui.qt.extapplication:OFF
  org.mitk.gui.qt.common:ON
  org.mitk.gui.qt.stdmultiwidgeteditor:ON
  org.mitk.gui.qt.common.legacy:OFF
  org.mitk.gui.qt.cmdlinemodules:OFF
  org.mitk.gui.qt.diffusionimagingapp:OFF
  org.mitk.gui.qt.datamanager:ON
  org.mitk.gui.qt.datamanagerlight:OFF
  org.mitk.gui.qt.properties:ON
  org.mitk.gui.qt.basicimageprocessing:OFF
  org.mitk.gui.qt.dicom:OFF
  org.mitk.gui.qt.diffusionimaging:OFF
  org.mitk.gui.qt.dtiatlasapp:OFF
  org.mitk.gui.qt.examples:OFF
  org.mitk.gui.qt.examplesopencv:OFF
  org.mitk.gui.qt.igtexamples:OFF
  org.mitk.gui.qt.igttracking:OFF
  org.mitk.gui.qt.imagecropper:OFF
  org.mitk.gui.qt.imagenavigator:ON
  org.mitk.gui.qt.materialeditor:OFF
  org.mitk.gui.qt.measurementtoolbox:OFF
  org.mitk.gui.qt.meshdecimation:OFF
  org.mitk.gui.qt.moviemaker:OFF
  org.mitk.gui.qt.pointsetinteraction:OFF
  org.mitk.gui.qt.python:OFF
  org.mitk.gui.qt.registration:OFF
  org.mitk.gui.qt.remeshing:OFF
  org.mitk.gui.qt.segmentation:OFF
  org.mitk.gui.qt.simulation:OFF
  org.mitk.gui.qt.toftutorial:OFF
  org.mitk.gui.qt.tofutil:OFF
  org.mitk.gui.qt.ugvisualization:OFF
  org.mitk.gui.qt.ultrasound:OFF
  org.mitk.gui.qt.volumevisualization:OFF
)

else()

set(MITK_EXT_PLUGINS
  # empty so far
)

endif()
