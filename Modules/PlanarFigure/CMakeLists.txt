MITK_CREATE_MODULE( PlanarFigure
  INCLUDE_DIRS Algorithms DataManagement Interactions IO Rendering
  DEPENDS Mitk SceneSerializationBase
)

ADD_SUBDIRECTORY(Testing)
