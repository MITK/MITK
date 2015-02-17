MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE src/Algorithms src/DataStructure src/Interactions src/Rendering src/IO
  DEPENDS MitkSceneSerializationBase
  PACKAGE_DEPENDS Boost
  #WARNINGS_AS_ERRORS
)

#add_subdirectory(test)
