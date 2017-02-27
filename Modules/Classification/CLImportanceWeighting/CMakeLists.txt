MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkCLCore MitkDataCollection
  PACKAGE_DEPENDS PRIVATE Vigra
  WARNINGS_AS_ERRORS
)

#add_subdirectory(test)
