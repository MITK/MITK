MITK_CREATE_MODULE(
  DEPENDS MitkIGT
  PACKAGE_DEPENDS PUBLIC OpenCV
  WARNINGS_AS_ERRORS
)

# add testing dir
add_subdirectory(Testing)
