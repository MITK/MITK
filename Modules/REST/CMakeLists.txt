mitk_create_module(
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC cpprestsdk OpenSSL|SSL
)

if(TARGET ${MODULE_TARGET})
  add_subdirectory(test)
endif()
