mitk_create_module(MultilabelIO
  DEPENDS PUBLIC MitkMultilabel MitkSceneSerialization
  PACKAGE_DEPENDS PRIVATE ITK|IONRRD nlohmann_json
  AUTOLOAD_WITH MitkCore
)
