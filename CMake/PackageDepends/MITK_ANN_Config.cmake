# first look for the superbuild ANN version, pointed to by ANN_DIR
find_package(ANN QUIET CONFIG)
if(NOT ANN_FOUND)
  # now try invoking a FindANN.cmake script
  find_package(ANN REQUIRED)
endif()

list(APPEND ALL_LIBRARIES ${ANN_LIBRARIES})
if(ANN_INCLUDE_DIRS)
  list(APPEND ALL_INCLUDE_DIRECTORIES ${ANN_INCLUDE_DIRS})
endif()
