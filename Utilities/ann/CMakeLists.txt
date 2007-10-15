PROJECT(ANN)

OPTION(ANN_SHARED "Build ANN as shared lib" OFF)
MARK_AS_ADVANCED(ANN_SHARED)

SET(ANN_HEADERS
  include/ANN/ANN.h
  include/ANN/ANNperf.h
  include/ANN/ANNx.h
)
  
SET(ANN_SOURCES
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

INCLUDE_DIRECTORIES(${ANN_SOURCE_DIR}/include)
IF(ANN_SHARED)
  ADD_LIBRARY(ann SHARED ${ANN_HEADERS} ${ANN_SOURCES})
ELSE(ANN_SHARED)
  ADD_DEFINITIONS( -DANN_STATIC_LIB ) 
  ADD_LIBRARY(ann ${ANN_HEADERS} ${ANN_SOURCES})
ENDIF(ANN_SHARED)
