MITK_CREATE_MODULE(mitkCameraCalibration
  DEPENDS MitkIGT
  PACKAGE_DEPENDS OpenCV
)

# add testing dir
ADD_SUBDIRECTORY(Testing)