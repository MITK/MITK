MITK_CREATE_MODULE( SceneSerialization
  INCLUDE_DIRS BaseDataSerializer BasePropertySerializer BasePropertyDeserializer
  DEPENDS SceneSerializationBase LegacyIO
  PACKAGE_DEPENDS Poco
)

add_subdirectory(Testing)
