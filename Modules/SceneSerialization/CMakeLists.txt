MITK_CREATE_MODULE( SceneSerialization
  INCLUDE_DIRS BaseDataSerializer BasePropertySerializer BasePropertyDeserializer
  DEPENDS Mitk MitkExt SceneSerializationBase
  PACKAGE_DEPENDS Poco
)

add_subdirectory(Testing)
