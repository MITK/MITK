MITK_CREATE_MODULE( Persistence
  INCLUDE_DIRS
  DEPENDS SceneSerialization
  AUTOLOAD_WITH Mitk
)
add_subdirectory(Testing)
