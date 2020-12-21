foreach(python3_component ${Python3_REQUIRED_COMPONENTS_BY_MODULE})
  list(APPEND ALL_LIBRARIES "Python3::${python3_component}")
endforeach()
