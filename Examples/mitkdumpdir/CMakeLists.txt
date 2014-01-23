project(mitkdumpdir)
find_package(MITK REQUIRED)

mitk_check_module_dependencies(MODULES Qmitk MISSING_DEPENDENCIES_VAR _missing_deps)
if(_missing_deps)
  message(STATUS "mitkdumpdir application won't be built. Missing: ${_missing_deps}")
else(_missing_deps)

  mitk_create_executable(mitkdumpdir DEPENDS Qmitk WARNINGS_AS_ERRORS)

endif()
