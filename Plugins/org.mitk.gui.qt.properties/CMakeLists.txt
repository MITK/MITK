project(org_mitk_gui_qt_properties)

mitk_create_plugin(
  EXPORT_DIRECTIVE PROPERTIES_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
