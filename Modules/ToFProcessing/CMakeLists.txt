MITK_CREATE_MODULE(
    SUBPROJECTS MITK-ToF
    DEPENDS MitkCameraCalibration
    PACKAGE_DEPENDS OpenCV
  )
  
if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)

