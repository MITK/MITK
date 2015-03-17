set(MODULE_TESTS
  mitkComputeContourSetNormalsFilterTest.cpp
  mitkImageToPointCloudFilterTest.cpp
  mitkPointCloudScoringFilterTest
  mitkReduceContourSetFilterTest.cpp
  mitkSurfaceInterpolationControllerTest.cpp
)

if(NOT WIN32)
  set(MODULE_TESTS ${MODULE_TESTS} mitkCreateDistanceImageFromSurfaceFilterTest.cpp)
endif()
