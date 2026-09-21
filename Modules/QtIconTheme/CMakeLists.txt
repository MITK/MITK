mitk_create_module(
  NO_INIT
  AUTOMOC
  DEPENDS PRIVATE MitkLog
  PACKAGE_DEPENDS
    PUBLIC Qt6|Widgets
    PRIVATE Qt6|Svg
)
