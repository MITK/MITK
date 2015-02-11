MITK_CREATE_MODULE(
  DEPENDS MitkAlgorithmsExt MitkContourModel
  PACKAGE_DEPENDS Eigen
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
