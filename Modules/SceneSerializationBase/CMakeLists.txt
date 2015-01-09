MITK_CREATE_MODULE(
  INCLUDE_DIRS PUBLIC BasePropertySerializer
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC tinyxml
  WARNINGS_AS_ERRORS
)
add_subdirectory(Testing)

