set(_module_deps MitkFiberTracking )

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS include/
  DEPENDS ${_module_deps}
  PACKAGE_DEPENDS PUBLIC OpenCV Caffe2
)

add_subdirectory(cmdapps)

if(MODULE_IS_ENABLED)
  add_subdirectory(Testing)
endif()
