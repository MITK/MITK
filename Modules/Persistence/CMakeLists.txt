MITK_CREATE_MODULE( Persistence
  DEPENDS SceneSerialization
  AUTOLOAD_WITH SceneSerialization
)
add_subdirectory(Testing)
