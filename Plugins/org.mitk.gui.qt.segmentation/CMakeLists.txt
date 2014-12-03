project(org_mitk_gui_qt_segmentation)

include_directories(${CTK_INCLUDE_DIRS})

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE MITK_QT_SEGMENTATION
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkSegmentationUI
  PACKAGE_DEPENDS Qt4|QtCore Qt5|OpenGL
)
