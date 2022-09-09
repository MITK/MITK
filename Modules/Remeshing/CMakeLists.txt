mitk_create_module(
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS ACVD|Surface
  WARNINGS_NO_ERRORS # ACVD's header files trigger some unused parameter errors
)
