set(package_deps PACKAGE_DEPENDS PRIVATE)
if(NOT APPLE)
  list(APPEND package_deps SimpleITK)
endif()

MITK_CREATE_MODULE_TESTS(${package_deps})
