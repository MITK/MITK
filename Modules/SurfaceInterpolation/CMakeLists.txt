MITK_CREATE_MODULE(
  DEPENDS MitkImageExtraction MitkContourModel MitkAlgorithmsExt MitkImageStatistics
  PACKAGE_DEPENDS PUBLIC Eigen
  #DEPENDS MitkAlgorithmsExt MitkContourModel
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
