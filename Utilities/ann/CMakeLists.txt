PROJECT(ANN)

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
IF(NOT BUILD_SHARED_LIBS)
  ADD_DEFINITIONS( -DANN_STATIC_LIB )
ENDIF()
ADD_LIBRARY(ann ${ANN_HEADERS} ${ANN_SOURCES})

# install only if shared lib (for now)
INSTALL(TARGETS ann${LIBPOSTFIX}
   RUNTIME DESTINATION bin   
)