project(org_mitk_gui_qt_client)

mitk_create_plugin(
  EXPORT_DIRECTIVE THREAD_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgets MitkCppRestSdk
)
