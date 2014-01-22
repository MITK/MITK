project(mitkdump)
find_package(MITK)

mitk_check_module_dependencies(MODULES Qmitk DICOMReader MISSING_DEPENDENCIES_VAR _missing_deps)
if(_missing_deps)
  message(STATUS "mitkdump application won't be built. Missing: ${_missing_deps}")
else(_missing_deps)

  # couldn't find a differnt solution
  # adding "PACKAGES GDCM" to mitk_use_modules will not fix the problem
  # ld seems not to get GDCM_LIBRARY_DIRS into its search path
  # a (too?) simple addition to mitkFunctionUseModules does also not work
  find_package(GDCM PATHS ${GDCM_DIR} REQUIRED)
  link_directories(${GDCM_LIBRARY_DIRS})

  add_executable(mitkdump mitkdump.cpp)
  mitk_use_modules(TARGET mitkdump MODULES DICOMReader Qmitk)
  set_property(TARGET mitkdump PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})

  add_executable(mitkdumpdir mitkdumpdir.cpp)
  mitk_use_modules(TARGET mitkdumpdir MODULES DICOMReader Qmitk)
  set_property(TARGET mitkdumpdir PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})

  foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
    add_dependencies(${subproject} mitkdump)
    add_dependencies(${subproject} mitkdumpdir)
  endforeach()

endif()
