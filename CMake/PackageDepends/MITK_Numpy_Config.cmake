if(NOT MITK_USE_SYSTEM_PYTHON)
  set(Numpy_INCLUDE_DIR ${Numpy_DIR}/core/include)
  list(APPEND ALL_INCLUDE_DIRECTORIES ${Numpy_INCLUDE_DIR})
endif()
