if(MITK_USE_Caffe2 AND MITK_USE_Caffe)

  set(_module_deps MitkFiberTracking )

  MITK_CREATE_MODULE(
    SUBPROJECTS MITK-DTI
    INCLUDE_DIRS include/
    DEPENDS ${_module_deps}
    PACKAGE_DEPENDS PUBLIC OpenCV Caffe Caffe2
  )

  if(MODULE_IS_ENABLED)
    add_subdirectory(cmdapps)
    add_subdirectory(Testing)
  endif()

endif()
