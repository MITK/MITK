MITK_CREATE_MODULE(
  DEPENDS MitkCLCore MitkCLUtilities
  PACKAGE_DEPENDS
  #DEPENDS MitkClassificationCore MitkClassificationUtilities
)

add_subdirectory(test)
