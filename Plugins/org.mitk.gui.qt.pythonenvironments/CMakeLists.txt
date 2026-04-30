project(org_mitk_gui_qt_pythonenvironments)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_PYTHONENVIRONMENTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkPythonHelper
)
