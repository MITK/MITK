
# Plug-ins must be ordered according to their dependencies

set(MITK_PLUGINS

  org.blueberry.core.runtime:ON
  org.blueberry.core.expressions:OFF
  org.blueberry.core.commands:OFF
  org.blueberry.core.jobs:OFF
  org.blueberry.ui.qt:OFF
  org.blueberry.ui.qt.help:OFF
  org.blueberry.ui.qt.log:ON
  org.blueberry.ui.qt.objectinspector:OFF

  #org.blueberry.test:ON
  #org.blueberry.uitest:ON

  #Testing/org.blueberry.core.runtime.tests:ON
  #Testing/org.blueberry.osgi.tests:ON

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
  org.mitk.gui.qt.geometrytools:OFF
  org.mitk.gui.qt.igtexamples:OFF
  org.mitk.gui.qt.igttracking:OFF
  org.mitk.gui.qt.igtlplugin:OFF
  org.mitk.gui.qt.imagecropper:OFF
  org.mitk.gui.qt.imagenavigator:ON
  org.mitk.gui.qt.viewnavigator:OFF
  org.mitk.gui.qt.materialeditor:OFF
  org.mitk.gui.qt.measurementtoolbox:OFF
  org.mitk.gui.qt.moviemaker:OFF
  org.mitk.gui.qt.pointsetinteraction:OFF
  org.mitk.gui.qt.python:OFF
  org.mitk.gui.qt.registration:OFF
  org.mitk.gui.qt.remeshing:OFF
  org.mitk.gui.qt.segmentation:OFF
  org.mitk.gui.qt.simulation:OFF
  org.mitk.gui.qt.aicpregistration:OFF
  org.mitk.gui.qt.toftutorial:OFF
  org.mitk.gui.qt.tofutil:OFF
  org.mitk.gui.qt.tubegraph:OFF
  org.mitk.gui.qt.ugvisualization:OFF
  org.mitk.gui.qt.ultrasound:OFF
  org.mitk.gui.qt.volumevisualization:OFF
  org.mitk.gui.qt.eventrecorder:OFF
  org.mitk.gui.qt.xnat:OFF
)
