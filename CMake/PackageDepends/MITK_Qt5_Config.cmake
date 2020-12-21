find_package(Qt5 COMPONENTS ${Qt5_REQUIRED_COMPONENTS_BY_MODULE} REQUIRED QUIET)

foreach(qt5_component ${Qt5_REQUIRED_COMPONENTS_BY_MODULE})
  list(APPEND ALL_LIBRARIES Qt5::${qt5_component})
endforeach()
