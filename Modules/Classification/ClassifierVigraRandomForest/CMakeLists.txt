MITK_CREATE_MODULE(
  DEPENDS MitkClassificationCore
  PACKAGE_DEPENDS
    PRIVATE Vigra
  WARNINGS_AS_ERRORS
)

add_subdirectory(test)
