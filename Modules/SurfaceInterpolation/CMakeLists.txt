MITK_CREATE_MODULE(
  DEPENDS MitkContourModel MitkAlgorithmsExt MitkImageStatistics
  PACKAGE_DEPENDS PUBLIC ITK|ITKEigen3
)

add_subdirectory(Testing)
