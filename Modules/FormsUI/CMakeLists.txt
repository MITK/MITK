mitk_create_module(
  INCLUDE_DIRS
    PUBLIC include
    PRIVATE src
  DEPENDS
    PUBLIC MitkForms
  PACKAGE_DEPENDS
    PUBLIC Qt6|Widgets
)
