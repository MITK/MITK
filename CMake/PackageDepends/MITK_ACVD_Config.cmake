foreach(acvd_component ${ACVD_REQUIRED_COMPONENTS_BY_MODULE})
  if(NOT acvd_component MATCHES "^vtk")
    set(acvd_component "vtk${acvd_component}")
  endif()
  list(APPEND _acvd_required_components_by_module ${acvd_component})
endforeach()

find_package(ACVD COMPONENTS ${_acvd_required_components_by_module} REQUIRED)

# Route ACVD's include dirs through mitkFunctionUseModules, which marks
# them as SYSTEM. Without this the directory-scope include_directories()
# call inside ACVDConfig.cmake would leak non-SYSTEM entries to consumers
# and re-enable warnings that ACVD's public headers trip.
list(APPEND ALL_INCLUDE_DIRECTORIES ${ACVD_INCLUDE_DIRS})

foreach(acvd_component ${_acvd_required_components_by_module})
  list(APPEND ALL_LIBRARIES ${acvd_component})
endforeach()
