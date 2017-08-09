MITK_CREATE_MODULE(
  DEPENDS MitkSceneSerializationBase
  PACKAGE_DEPENDS PUBLIC Poco|Zip
)

add_subdirectory(test)
