set(H_FILES
  include/mitkRestSegmentation.h
  include/mitkRestSegmentationJobs.h
)

set(CPP_FILES
  mitkRestSegmentation.cpp
  mitkRestSegmentationJobs.cpp
)

if(TARGET MitkPythonSegmentation)
  list(APPEND H_FILES include/mitkRestPythonSegmentation.h)
  list(APPEND CPP_FILES mitkRestPythonSegmentation.cpp)
endif()
