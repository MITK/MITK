MITK_CREATE_MODULE( PlanarFigure
  INCLUDE_DIRS Algorithms DataManagement Interactions IO Rendering
  DEPENDS Mitk SceneSerialization
)

ADD_SUBDIRECTORY(Testing)
