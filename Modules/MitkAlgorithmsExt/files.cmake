set(CPP_FILES
  mitkAutoCropImageFilter.cpp
  mitkBoundingObjectCutter.cpp
  mitkBoundingObjectToSegmentationFilter.cpp
  mitkHeightFieldSurfaceClipImageFilter.cpp
  mitkMaskAndCutRoiImageFilter.cpp
  mitkMaskImageFilter.cpp
  mitkMovieGenerator.cpp
  mitkNonBlockingAlgorithm.cpp
  mitkPadImageFilter.cpp
  mitkSimpleHistogram.cpp
  mitkSimpleUnstructuredGridHistogram.cpp
)

if(WIN32 AND NOT MINGW)
  list(APPEND CPP_FILES
    mitkMovieGeneratorWin32.cpp
  )
endif()

