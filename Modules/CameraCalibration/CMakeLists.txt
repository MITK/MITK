MITK_CREATE_MODULE(
  DEPENDS MitkIGT
  PACKAGE_DEPENDS OpenCV
)

# add testing dir
add_subdirectory(Testing)
