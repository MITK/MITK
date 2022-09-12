MITK_CREATE_MODULE(
  DEPENDS MitkIGT
  PACKAGE_DEPENDS PUBLIC OpenCV|calib3d
)

# add testing dir
add_subdirectory(Testing)
