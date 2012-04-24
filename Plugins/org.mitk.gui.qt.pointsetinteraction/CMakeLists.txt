# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_gui_qt_pointsetinteraction)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE POINTSETINTERACTION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES QmitkExt
)