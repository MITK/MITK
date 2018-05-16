project(org_mitk_gui_qt_datamanagerlight)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_DATAMANAGER
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets
)
