mitk_create_module(
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS ACVD|Surface
)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
