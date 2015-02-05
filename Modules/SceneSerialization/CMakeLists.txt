MITK_CREATE_MODULE(
  INCLUDE_DIRS BaseDataSerializer
  DEPENDS MitkSceneSerializationBase
  PACKAGE_DEPENDS PUBLIC Poco|Zip
  WARNINGS_AS_ERRORS
)

add_subdirectory(test)
