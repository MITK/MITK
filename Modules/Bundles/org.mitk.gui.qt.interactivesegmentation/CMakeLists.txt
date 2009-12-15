# Deactivate for MacOS. Module will be replaced by org.mitk.gui.qt.segmentation soon.
IF (NOT APPLE)
  SET(QT_USE_QT3SUPPORT 1)

  MACRO_CREATE_MITK_PLUGIN(QmitkExt)
ENDIF()
