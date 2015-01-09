MITK_CREATE_MODULE(
  DEPENDS MitkImageExtraction
  PACKAGE_DEPENDS PUBLIC Eigen
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
