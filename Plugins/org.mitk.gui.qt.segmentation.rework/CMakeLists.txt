project(org_mitk_gui_qt_segmentation_rework)

mitk_create_plugin(
  EXPORT_DIRECTIVE REWORK_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkCppRestSdk MitkChart MitkSegmentationUI
)
