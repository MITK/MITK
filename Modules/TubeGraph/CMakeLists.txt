MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE src/Algorithms src/DataStructure src/Interactions src/Rendering src/IO
  DEPENDS MitkSceneSerializationBase
)

#add_subdirectory(test)
