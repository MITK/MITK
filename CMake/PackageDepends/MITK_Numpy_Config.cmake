if(MITK_USE_Python)
  find_package(Numpy REQUIRED)
  list(APPEND ALL_INCLUDE_DIRECTORIES ${NUMPY_INCLUDE_DIR})
endif()
