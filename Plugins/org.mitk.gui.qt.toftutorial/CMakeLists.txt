# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_toftutorial)

mitk_create_plugin(
  EXPORT_DIRECTIVE TOFTUTORIAL_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkToFHardware MitkToFProcessing
)
