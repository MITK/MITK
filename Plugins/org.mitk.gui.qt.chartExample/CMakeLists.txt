project(org_mitk_gui_qt_chartExample)

mitk_create_plugin(
  EXPORT_DIRECTIVE CHARTEXAMPLE_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkChart
)
