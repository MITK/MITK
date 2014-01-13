MITK_CREATE_MODULE( SceneSerializationBase
  INCLUDE_DIRS BaseDataSerializer BasePropertySerializer
  DEPENDS Mitk
  WARNINGS_AS_ERRORS
)
add_subdirectory(Testing)

