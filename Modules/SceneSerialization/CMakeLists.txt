MITK_CREATE_MODULE(
  DEPENDS MitkSceneSerializationBase
  PACKAGE_DEPENDS PUBLIC Poco|Zip
  WARNINGS_AS_ERRORS
)

add_subdirectory(test)
