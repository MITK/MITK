MITK_CREATE_MODULE(
  DEPENDS MitkCLCore MitkCLUtilities
  PACKAGE_DEPENDS
  #DEPENDS MitkClassificationCore MitkClassificationUtilities
  #PACKAGE_DEPENDS Boost
  WARNINGS_AS_ERRORS
)

add_subdirectory(test)
