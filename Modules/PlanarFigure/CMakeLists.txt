MITK_CREATE_MODULE(
  INCLUDE_DIRS PRIVATE src/Algorithms src/DataManagement src/Interactions src/Rendering
  DEPENDS MitkLegacyGL MitkAnnotation
)

add_subdirectory(autoload/IO)
if(BUILD_TESTING)
 add_subdirectory(test)
endif()

