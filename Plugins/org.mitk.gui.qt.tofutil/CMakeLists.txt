# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_tofutil)

mitk_create_plugin(
  EXPORT_DIRECTIVE TOFUTIL_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkToFHardware MitkQtWidgetsExt MitkToFUI
)


