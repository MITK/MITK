set(_module_deps MitkFiberTracking )

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms
  DEPENDS ${_module_deps}
  PACKAGE_DEPENDS PUBLIC Caffe2
)

add_subdirectory(MiniApps)
