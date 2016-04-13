mitk_create_module(
  SUBPROJECTS MITK-IGT
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC OpenIGTLink
  INCLUDE_DIRS Filters DeviceSources
  #WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)

