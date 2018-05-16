MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE src/Algorithms src/DataManagement src/Interactions src/IO src/Rendering
  DEPENDS MitkSceneSerializationBase MitkLegacyGL MitkAnnotation
)

add_subdirectory(test)

