MITK_CREATE_MODULE( SceneSerialization
  INCLUDE_DIRS BaseDataSerializer BasePropertySerializer BasePropertyDeserializer
  DEPENDS Mitk MitkExt SceneSerializationBase
  PACKAGE_DEPENDS Poco
  ADDITIONAL_LIBS PocoFoundation PocoZip
)

ADD_SUBDIRECTORY(Testing)
