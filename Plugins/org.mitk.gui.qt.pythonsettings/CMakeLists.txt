project(org_mitk_gui_qt_pythonsettings)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_PYTHONSETTINGS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkPythonHelper
)
