MITK_CREATE_MODULE(
  DEPENDS MitkImageExtraction MitkContourModel MitkAlgorithmsExt MitkImageStatistics
  PACKAGE_DEPENDS PUBLIC ITK|ITKEigen3
)

add_subdirectory(Testing)
