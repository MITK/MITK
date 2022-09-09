foreach(acvd_component ${ACVD_REQUIRED_COMPONENTS_BY_MODULE})
  if(NOT acvd_component MATCHES "^vtk")
    set(acvd_component "vtk${acvd_component}")
  endif()
  list(APPEND _acvd_required_components_by_module ${acvd_component})
endforeach()

find_package(ACVD COMPONENTS ${_acvd_required_components_by_module} REQUIRED)

foreach(acvd_component ${_acvd_required_components_by_module})
  list(APPEND ALL_LIBRARIES ${acvd_component})
endforeach()
