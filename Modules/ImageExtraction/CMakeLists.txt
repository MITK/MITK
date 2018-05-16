MITK_CREATE_MODULE(
  DEPENDS MitkAlgorithmsExt
  PACKAGE_DEPENDS PRIVATE ITK|ITKIOImageBase
)

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)
