# Deactivate for all platforms to remove QT3SUPPORT dependency. Module is replaced by org.mitk.gui.qt.segmentation
#IF (NOT APPLE)
#  SET(QT_USE_QT3SUPPORT 1)
#
#  MACRO_CREATE_MITK_PLUGIN(QmitkExt)
#ENDIF()
