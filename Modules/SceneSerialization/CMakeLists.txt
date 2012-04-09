MITK_CREATE_MODULE( SceneSerialization
  INCLUDE_DIRS BaseDataSerializer BasePropertySerializer BasePropertyDeserializer
  DEPENDS Mitk MitkExt SceneSerializationBase
  PACKAGE_DEPENDS Poco
  ADDITIONAL_LIBS optimized PocoFoundation debug PocoFoundationd optimized PocoZip debug PocoZipd
)

add_subdirectory(Testing)
