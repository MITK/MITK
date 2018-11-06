mitk_create_module(
  PACKAGE_DEPENDS
    PUBLIC CTK|CTKPluginFramework Qt5|Widgets Poco|Util
  DEPENDS
    PUBLIC qtsingleapplication MitkUtilities
    PRIVATE MitkCore
  WARNINGS_AS_ERRORS
)
