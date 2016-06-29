# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_example_gui_opencv)

mitk_create_plugin(
  EXPORT_DIRECTIVE EXAMPLESOPENCV_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkOpenCVVideoSupportUI MitkPersistence
)
