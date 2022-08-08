MITK_CREATE_MODULE(
    DEPENDS MitkCameraCalibration
    WARNINGS_NO_ERRORS
  )

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)

