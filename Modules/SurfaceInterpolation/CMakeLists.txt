MITK_CREATE_MODULE(
  DEPENDS MitkImageExtraction MitkContourModel MitkAlgorithmsExt MitkImageStatistics
  PACKAGE_DEPENDS PUBLIC Eigen
)

add_subdirectory(Testing)
