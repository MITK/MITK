MITK_CREATE_MODULE(
  DEPENDS MitkAlgorithmsExt
  WARNINGS_AS_ERRORS
)

if(BUILD_TESTING)

  add_subdirectory(Testing)

endif(BUILD_TESTING)
