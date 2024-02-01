find_package(Qt6 COMPONENTS ${Qt6_REQUIRED_COMPONENTS_BY_MODULE} REQUIRED QUIET)

foreach(qt6_component ${Qt6_REQUIRED_COMPONENTS_BY_MODULE})
  list(APPEND ALL_LIBRARIES Qt6::${qt6_component})
endforeach()
