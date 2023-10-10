mitk_create_module(ROIIO
  DEPENDS PUBLIC MitkROI MitkSceneSerialization
  PACKAGE_DEPENDS PRIVATE nlohmann_json
  AUTOLOAD_WITH MitkCore
)
