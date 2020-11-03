project(org_mitk_gui_qt_flow_segmentation)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_GUI_QT_FLOW_SEGMENTATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkMultilabel
)
