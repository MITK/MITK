project(org_mitk_gui_qt_segmentation_helper)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_SEGMENTATION_HELPER
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkMultilabel
)
