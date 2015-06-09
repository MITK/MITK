project(org_mitk_gui_qt_ugvisualization)

mitk_create_plugin(
  EXPORT_DIRECTIVE UGVISUALIZATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
