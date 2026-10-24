MITK_CREATE_MODULE( PlanarFigureIO
  DEPENDS MitkPlanarFigure MitkSceneSerializationBase
  PACKAGE_DEPENDS PRIVATE nlohmann_json tinyxml2
  AUTOLOAD_WITH MitkCore
)
