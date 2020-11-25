find_package(Boost REQUIRED COMPONENTS ${Boost_REQUIRED_COMPONENTS_BY_MODULE})

if(Boost_REQUIRED_COMPONENTS_BY_MODULE)
  foreach(boost_component ${Boost_REQUIRED_COMPONENTS_BY_MODULE})
    list(APPEND ALL_LIBRARIES "Boost::${boost_component}")
  endforeach()
endif()

list(APPEND ALL_LIBRARIES "Boost::boost")

if(MSVC)
  list(APPEND ALL_LIBRARIES "Boost::dynamic_linking" "bcrypt")
endif()
