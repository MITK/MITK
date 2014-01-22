MITK_CREATE_MODULE( SceneSerialization
  INCLUDE_DIRS BaseDataSerializer BasePropertySerializer BasePropertyDeserializer
  DEPENDS SceneSerializationBase
  PACKAGE_DEPENDS Poco
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
