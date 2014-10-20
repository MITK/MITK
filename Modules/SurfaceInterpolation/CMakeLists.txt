MITK_CREATE_MODULE(
  DEPENDS MitkImageExtraction
  PACKAGE_DEPENDS Eigen
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
