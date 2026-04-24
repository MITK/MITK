MITK_CREATE_MODULE(
  DEPENDS MitkSceneSerializationBase
  PACKAGE_DEPENDS
    PUBLIC Poco|Zip
    PRIVATE nlohmann_json
)

add_subdirectory(autoload/IO)
add_subdirectory(test)
