set(H_FILES
  itkIntelligentBinaryClosingFilter.h
  mitkBoundingObjectCutAndCast.h
  mitkMovieGeneratorWin32.h
  mitkNonBlockingAlgorithmEvents.h
)

set(CPP_FILES
  mitkAutoCropImageFilter.cpp
  mitkBoundingObjectCutter.cpp
  mitkBoundingObjectToSegmentationFilter.cpp
  mitkGeometryClipImageFilter.cpp
  mitkGeometryDataSource.cpp
  mitkHeightFieldSurfaceClipImageFilter.cpp
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
  mitkCovarianceMatrixCalculator.cpp
  mitkAnisotropicIterativeClosestPointRegistration.cpp
  mitkWeightedPointTransform.cpp
  mitkAnisotropicRegistrationCommon.cpp
  mitkSurfaceToPointSetFilter.cpp
  mitkCropTimestepsImageFilter.cpp
)

if(WIN32)
  list(APPEND CPP_FILES
    mitkMovieGeneratorWin32.cpp
  )
endif()
