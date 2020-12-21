if(NOT Poco_REQUIRED_COMPONENTS_BY_MODULE)
  set(Poco_REQUIRED_COMPONENTS_BY_MODULE Foundation)
endif()

find_package(Poco PATHS ${Poco_DIR} COMPONENTS ${Poco_REQUIRED_COMPONENTS_BY_MODULE} CONFIG REQUIRED)

foreach(poco_component ${Poco_REQUIRED_COMPONENTS_BY_MODULE})
  list(APPEND ALL_LIBRARIES "Poco::${poco_component}")
endforeach()
