# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_example_gui_opencv)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE EXAMPLESOPENCV_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS mitkOpenCVVideoSupportUI
)
