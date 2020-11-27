set(boost_depends "Boost|date_time+regex+system")

if(UNIX)
  set(boost_depends "${boost_depends}+atomic+chrono+filesystem+random+thread")
endif()

mitk_create_module(
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC cpprestsdk OpenSSL|SSL ${boost_depends}
)

if(TARGET ${MODULE_TARGET})
  add_subdirectory(test)
endif()
