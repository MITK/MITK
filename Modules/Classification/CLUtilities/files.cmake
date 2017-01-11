file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkCLResultWritter.cpp

  Algorithms/itkLabelSampler.cpp
  Algorithms/itkSmoothedClassProbabilites.cpp
  Algorithms/mitkRandomImageSampler.cpp

  Features/itkNeighborhoodFunctorImageFilter.cpp
  Features/itkLineHistogramBasedMassImageFilter.cpp

  GlobalImageFeatures/mitkGIFCooccurenceMatrix.cpp
  GlobalImageFeatures/mitkGIFCooccurenceMatrix2.cpp
  GlobalImageFeatures/mitkGIFGrayLevelRunLength.cpp
  GlobalImageFeatures/mitkGIFGrayLevelSizeZone.cpp
  GlobalImageFeatures/mitkGIFFirstOrderStatistics.cpp
  GlobalImageFeatures/mitkGIFVolumetricStatistics.cpp
  GlobalImageFeatures/mitkGIFNeighbouringGreyLevelDependenceFeatures.cpp
  GlobalImageFeatures/mitkGIFNeighbourhoodGreyLevelDifference.cpp
  
  mitkCLUtil.cpp

)

set( TOOL_FILES
)
