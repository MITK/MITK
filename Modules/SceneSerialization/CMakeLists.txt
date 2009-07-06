MITK_CREATE_MODULE( SceneSerialization
  INCLUDE_DIRS BaseDataSerializer BasePropertySerializer
  DEPENDS Mitk 
  PACKAGE_DEPENDS Poco
  ADDITIONAL_LIBS PocoFoundation PocoZip
)

#ADD_SUBDIRECTORY(Testing)
