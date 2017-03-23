mitk_create_module(
  PACKAGE_DEPENDS
    PUBLIC CTK|CTKPluginFramework Qt5|Widgets Poco|Util Boost
  DEPENDS
    PUBLIC qtsingleapplication
    PRIVATE MitkCore
  WARNINGS_AS_ERRORS
)
