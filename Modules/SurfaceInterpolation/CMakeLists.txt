MITK_CREATE_MODULE(
  DEPENDS MitkAlgorithmsExt
  PACKAGE_DEPENDS PUBLIC Eigen
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
