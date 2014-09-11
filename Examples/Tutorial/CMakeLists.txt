include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(stepNo RANGE 1 8)
  add_subdirectory(Step${stepNo})
endforeach()
