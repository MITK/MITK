set(CPP_FILES
  mitkAutoCropImageFilter.cpp
  mitkBoundingObjectCutter.cpp
  mitkBoundingObjectToSegmentationFilter.cpp
  mitkGeometryClipImageFilter.cpp
  mitkHeightFieldSurfaceClipImageFilter.cpp
  mitkLabeledImageToSurfaceFilter.cpp
  mitkMaskAndCutRoiImageFilter.cpp
  mitkMaskImageFilter.cpp
  mitkMovieGenerator.cpp
  mitkNonBlockingAlgorithm.cpp
  mitkPadImageFilter.cpp
  mitkPlaneLandmarkProjector.cpp
  mitkPointLocator.cpp
  mitkSimpleHistogram.cpp
  mitkSimpleUnstructuredGridHistogram.cpp
)

if(WIN32 AND NOT MINGW)
  list(APPEND CPP_FILES
    mitkMovieGeneratorWin32.cpp
  )
endif()

