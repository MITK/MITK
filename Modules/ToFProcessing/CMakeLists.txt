MITK_CREATE_MODULE(
    SUBPROJECTS MITK-ToF
    DEPENDS MitkCameraCalibration
    PACKAGE_DEPENDS OpenCV
    WARNINGS_AS_ERRORS
  )
  
if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)

