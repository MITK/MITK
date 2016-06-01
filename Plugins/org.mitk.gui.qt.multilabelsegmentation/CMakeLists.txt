project(org_mitk_gui_qt_multilabelsegmentation)

include_directories(${CTK_INCLUDE_DIRS})

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_SEGMENTATION
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkSegmentation MitkSegmentationUI
)
