find_package(OpenSSL COMPONENTS ${OpenSSL_REQUIRED_COMPONENTS_BY_MODULE} REQUIRED)

foreach(openssl_component ${OpenSSL_REQUIRED_COMPONENTS_BY_MODULE})
  list(APPEND ALL_LIBRARIES "OpenSSL::${openssl_component}")
endforeach()
