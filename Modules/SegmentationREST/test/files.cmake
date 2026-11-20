set(MODULE_TESTS
  mitkRestSegmentationTest.cpp
  mitkRestSegmentationJobsTest.cpp
)

if(TARGET MitkPythonSegmentation)
  list(APPEND MODULE_TESTS mitkRestPythonSegmentationTest.cpp)
endif()
