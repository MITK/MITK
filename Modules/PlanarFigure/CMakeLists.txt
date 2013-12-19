MITK_CREATE_MODULE( PlanarFigure
  INCLUDE_DIRS Algorithms DataManagement Interactions IO Rendering
  DEPENDS Mitk LegacyIO SceneSerializationBase
)

IF( BUILD_TESTING )
add_subdirectory(Testing)
ENDIF()
