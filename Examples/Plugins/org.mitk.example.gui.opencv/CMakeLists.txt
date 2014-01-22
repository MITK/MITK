# The project name must correspond to the directory name of your plug-in
# and must not contain periods.
project(org_mitk_example_gui_opencv)

mitk_check_module_dependencies(MODULES mitkOpenCVVideoSupportUI MISSING_DEPENDENCIES_VAR _missing_deps)

if (_missing_deps)
  message(STATUS "org.mitk.example.gui.opencv plugin won't be built. Missing: ${_missing_deps}")
else(_missing_deps)
  MACRO_CREATE_MITK_CTK_PLUGIN(
    EXPORT_DIRECTIVE EXAMPLESOPENCV_EXPORTS
    EXPORTED_INCLUDE_SUFFIXES src
    MODULE_DEPENDS mitkOpenCVVideoSupportUI
  )
endif(_missing_deps)
