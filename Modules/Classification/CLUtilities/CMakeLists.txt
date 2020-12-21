mitk_create_module(
  DEPENDS MitkCore MitkCLCore MitkCommandLine MitkDICOM
  PACKAGE_DEPENDS PUBLIC Eigen OpenMP PRIVATE tinyxml2 VTK|FiltersStatistics
)

if(TARGET ${MODULE_TARGET})
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
