MITK_CREATE_MODULE( Persistence
  INCLUDE_DIRS
  DEPENDS SceneSerialization
  AUTOLOAD_WITH SceneSerialization
)
add_subdirectory(Testing)