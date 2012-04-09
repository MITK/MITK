MITK_CREATE_MODULE(mitkToFProcessing
    SUBPROJECTS MITK-ToF
    DEPENDS Mitk MitkExt mitkCameraCalibration
    PACKAGE_DEPENDS OpenCV
  )
  
if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)

