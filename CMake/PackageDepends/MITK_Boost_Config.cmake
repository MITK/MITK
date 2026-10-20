find_package(Boost CONFIG REQUIRED COMPONENTS ${Boost_REQUIRED_COMPONENTS_BY_MODULE})

if(Boost_REQUIRED_COMPONENTS_BY_MODULE)
  foreach(boost_component ${Boost_REQUIRED_COMPONENTS_BY_MODULE})
    list(APPEND ALL_LIBRARIES "Boost::${boost_component}")
  endforeach()

  if(MSVC)
    # Compiled Boost libraries are linked through their CMake targets. Boost's
    # auto-linking would ask for versioned file names the system layout lacks.
    list(APPEND ALL_LIBRARIES "Boost::disable_autolinking")
  endif()
endif()

list(APPEND ALL_LIBRARIES "Boost::boost")

if(MSVC)
  list(APPEND ALL_LIBRARIES "Boost::dynamic_linking" "bcrypt")
endif()
