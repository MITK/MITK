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
  #GlobalImageFeatures/itkEnhancedScalarImageToRunLengthFeaturesFilter.hxx
  #GlobalImageFeatures/itkEnhancedScalarImageToRunLengthMatrixFilter.hxx
  #GlobalImageFeatures/itkEnhancedHistogramToRunLengthFeaturesFilter.hxx
  #GlobalImageFeatures/itkEnhancedHistogramToTextureFeaturesFilter.hxx
  #GlobalImageFeatures/itkEnhancedScalarImageToTextureFeaturesFilter.hxx
  mitkCLUtil.cpp

)

set( TOOL_FILES
)
