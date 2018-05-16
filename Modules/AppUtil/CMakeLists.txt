mitk_create_module(
  PACKAGE_DEPENDS
    PUBLIC CTK|CTKPluginFramework Qt5|Widgets Poco|Util
    PRIVATE VTK
  DEPENDS
    PUBLIC qtsingleapplication
    PRIVATE MitkCore
)
