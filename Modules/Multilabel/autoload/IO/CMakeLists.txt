mitk_create_module(MultilabelIO
  DEPENDS PUBLIC MitkMultilabel MitkSceneSerialization
  PACKAGE_DEPENDS PRIVATE ITK|IONRRD
  AUTOLOAD_WITH MitkCore
)
