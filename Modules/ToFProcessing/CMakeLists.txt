MITK_CREATE_MODULE(
    DEPENDS MitkCameraCalibration
    PACKAGE_DEPENDS OpenCV
    WARNINGS_NO_ERRORS
  )

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)

