mitk_create_module(
  SUBPROJECTS MITK-IGT
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC OpenIGTLink
  INCLUDE_DIRS Filters DeviceSources
)

add_subdirectory(Testing)

