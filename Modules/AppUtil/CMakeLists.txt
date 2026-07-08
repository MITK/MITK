mitk_create_module(
  PACKAGE_DEPENDS
    PUBLIC CTK|CTKPluginFramework Qt6|Widgets Poco|Util
    PRIVATE VTK|GUISupportQt
  DEPENDS
    PUBLIC qtsingleapplication
    PRIVATE MitkCore
)
