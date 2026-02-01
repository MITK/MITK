project(org_mitk_gui_qt_restapi)

mitk_create_plugin(
  EXPORT_DIRECTIVE RESTAPIPLUGIN_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkRESTAPI MitkQtWidgets
)
