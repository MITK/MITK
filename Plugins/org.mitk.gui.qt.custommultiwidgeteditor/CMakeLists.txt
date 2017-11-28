project(org_mitk_gui_qt_custommultiwidgeteditor)

mitk_create_plugin(
  EXPORT_DIRECTIVE CUSTOMMULTIWIDGETEDITOR_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets
)
