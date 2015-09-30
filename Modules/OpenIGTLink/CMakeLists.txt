mitk_create_module(
  SUBPROJECTS MITK-IGT
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC OpenIGTLink
  INCLUDE_DIRS Filters
  #WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)

