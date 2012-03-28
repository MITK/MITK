MITK_CREATE_MODULE(mitkCameraCalibration
  DEPENDS MitkIGT
  PACKAGE_DEPENDS OpenCV
)

# add testing dir
add_subdirectory(Testing)