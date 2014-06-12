MITK_CREATE_MODULE(
  INCLUDE_DIRS BaseDataSerializer BasePropertySerializer BasePropertyDeserializer
  DEPENDS MitkSceneSerializationBase MitkLegacyIO
  PACKAGE_DEPENDS Poco
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
