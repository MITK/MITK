set(MODULE_TESTS
  mitkComputeContourSetNormalsFilterTest.cpp
  mitkPointCloudScoringFilterTest
  mitkReduceContourSetFilterTest.cpp
  mitkSurfaceInterpolationControllerTest.cpp
)

if(NOT WIN32)
  set(MODULE_TESTS ${MODULE_TESTS} mitkCreateDistanceImageFromSurfaceFilterTest.cpp)
endif()
