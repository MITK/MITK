MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkUtilities
  PACKAGE_DEPENDS PUBLIC tinyxml
  WARNINGS_AS_ERRORS
)
add_subdirectory(test)

