MITK_CREATE_MODULE(mitkCameraCalibration
  DEPENDS MitkIGT
  PACKAGE_DEPENDS OpenCV
  PROVIDES mitkCameraCalibration
)

# add testing dir
ADD_SUBDIRECTORY(Testing)