file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkAutoCropImageFilter.cpp
  mitkBoundingObjectCutter.cpp
  mitkBoundingObjectToSegmentationFilter.cpp
  mitkGeometryClipImageFilter.cpp
  mitkGeometryDataSource.cpp
  mitkHeightFieldSurfaceClipImageFilter.cpp
  mitkImageToUnstructuredGridFilter.cpp
  mitkLabeledImageToSurfaceFilter.cpp
  mitkMaskAndCutRoiImageFilter.cpp
  mitkMaskImageFilter.cpp
  mitkMovieGenerator.cpp
  mitkNonBlockingAlgorithm.cpp
  mitkPadImageFilter.cpp
  mitkPlaneFit.cpp
  mitkPlaneLandmarkProjector.cpp
  mitkPointLocator.cpp
  mitkSegmentationSink.cpp
  mitkSimpleHistogram.cpp
  mitkSimpleUnstructuredGridHistogram.cpp
  mitkCovarianceMatrixCalculator.cpp
  mitkAnisotropicIterativeClosestPointRegistration.cpp
  mitkWeightedPointTransform.cpp
  mitkAnisotropicRegistrationCommon.cpp
  mitkUnstructuredGridClusteringFilter.cpp
  mitkUnstructuredGridToUnstructuredGridFilter.cpp
  mitkSurfaceToPointSetFilter.cpp
  mitkCropTimestepsImageFilter.cpp
)

if(WIN32)
  list(APPEND CPP_FILES
    mitkMovieGeneratorWin32.cpp
  )
endif()
