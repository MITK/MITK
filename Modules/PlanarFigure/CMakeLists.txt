MITK_CREATE_MODULE(
  INCLUDE_DIRS Algorithms DataManagement Interactions IO Rendering
  DEPENDS MitkLegacyIO MitkSceneSerializationBase MitkLegacyGL
  WARNINGS_AS_ERRORS
)

IF( BUILD_TESTING )
add_subdirectory(Testing)
ENDIF()
