project(ANN)

set(ANN_HEADERS
  include/ANN/ANN.h
  include/ANN/ANNperf.h
  include/ANN/ANNx.h
)
  
set(ANN_SOURCES
  src/ANN.cpp
  src/bd_fix_rad_search.cpp
  src/bd_pr_search.cpp
  src/bd_search.cpp
  src/bd_tree.cpp
  src/brute.cpp
  src/kd_dump.cpp
  src/kd_fix_rad_search.cpp
  src/kd_pr_search.cpp
  src/kd_search.cpp
  src/kd_split.cpp
  src/kd_tree.cpp
  src/kd_util.cpp
  src/perf.cpp
)

include_directories(${ANN_SOURCE_DIR}/include)
if(NOT BUILD_SHARED_LIBS)
  add_definitions( -DANN_STATIC_LIB )
endif()
add_library(ann ${ANN_HEADERS} ${ANN_SOURCES})

MITK_CREATE_MODULE_CONF(ann
  INCLUDE_DIRS include
)
