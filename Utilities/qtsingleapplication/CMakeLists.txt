mitk_create_module(
  PACKAGE_DEPENDS
    PUBLIC Qt4|QtGui Qt5|Widgets
    PRIVATE Qt4|QtNetwork Qt5|Network
  FORCE_STATIC
  NO_INIT
)
