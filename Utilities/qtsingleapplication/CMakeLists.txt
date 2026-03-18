mitk_create_module(
  AUTOMOC
  PACKAGE_DEPENDS
    PUBLIC Qt6|Widgets
    PRIVATE Qt6|Network
  FORCE_STATIC
  NO_INIT
)
