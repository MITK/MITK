# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
PROJECT(org_mitk_gui_qt_examplesopencv)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE EXAMPLESOPENCV_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES mitkOpenCVVideoSupportUI
)