MITK_CREATE_MODULE(
  DEPENDS MitkCLCore MitkCLUtilities
  PACKAGE_DEPENDS
  #DEPENDS MitkClassificationCore MitkClassificationUtilities
  WARNINGS_AS_ERRORS
)

add_subdirectory(test)
