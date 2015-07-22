file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  Algorithms/itkLabelSampler.cpp
  Algorithms/itkSmoothedClassProbabilites.cpp

  Features/itkNeighborhoodFunctorImageFilter.cpp
  Features/itkLineHistogramBasedMassImageFilter.cpp

  GlobalImageFeatures/mitkGIFCooccurenceMatrix.cpp
  GlobalImageFeatures/mitkGIFGrayLevelRunLength.cpp
  GlobalImageFeatures/mitkGIFFirstOrderStatistics.cpp
  GlobalImageFeatures/mitkGIFVolumetricStatistics.cpp
  mitkCLUtil.cpp

)

set( TOOL_FILES
)
