set(_module_deps MitkDeepDiffusion )

mitk_check_module_dependencies(
  MODULES ${_module_deps}
  MISSING_DEPENDENCIES_VAR _missing_deps
)

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS
  DEPENDS ${_module_deps}
  PACKAGE_DEPENDS PUBLIC Caffe2
)

add_subdirectory(MiniApps)
