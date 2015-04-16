set(package_deps PACKAGE_DEPENDS PRIVATE)
list(APPEND package_deps SimpleITK)


MITK_CREATE_MODULE_TESTS(${package_deps})
