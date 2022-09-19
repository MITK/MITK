foreach(gdcm_component ${GDCM_REQUIRED_COMPONENTS_BY_MODULE})
  if(NOT gdcm_component MATCHES "^gdcm")
    set(gdcm_component "gdcm${gdcm_component}")
  endif()
  list(APPEND _gdcm_required_components_by_module ${gdcm_component})
endforeach()

find_package(GDCM COMPONENTS ${_gdcm_required_components_by_module} REQUIRED)

list(APPEND ALL_LIBRARIES ${_gdcm_required_components_by_module})
