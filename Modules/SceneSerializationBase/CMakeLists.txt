MITK_CREATE_MODULE( SceneSerializationBase
  INCLUDE_DIRS BaseDataSerializer BasePropertySerializer BasePropertyDeserializer
  DEPENDS Mitk
  PACKAGE_DEPENDS Poco
  ADDITIONAL_LIBS PocoFoundation PocoZip
)

ADD_SUBDIRECTORY(Testing)

