MITK_CREATE_MODULE(
  DEPENDS MitkIGT
  PACKAGE_DEPENDS PUBLIC OpenCV
)

# add testing dir
add_subdirectory(Testing)
