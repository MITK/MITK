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
  GlobalImageFeatures/mitkGIFGreyLevelRunLength.cpp
  GlobalImageFeatures/mitkGIFImageDescriptionFeatures.cpp
  GlobalImageFeatures/mitkGIFFirstOrderStatistics.cpp
  GlobalImageFeatures/mitkGIFFirstOrderHistogramStatistics.cpp
  GlobalImageFeatures/mitkGIFFirstOrderNumericStatistics.cpp
  GlobalImageFeatures/mitkGIFVolumetricStatistics.cpp
  GlobalImageFeatures/mitkGIFNeighbouringGreyLevelDependenceFeatures.cpp
  GlobalImageFeatures/mitkGIFNeighbourhoodGreyLevelDifference.cpp
  GlobalImageFeatures/mitkGIFGreyLevelSizeZone.cpp
  GlobalImageFeatures/mitkGIFGreyLevelDistanceZone.cpp
  GlobalImageFeatures/mitkGIFLocalIntensity.cpp
  GlobalImageFeatures/mitkGIFVolumetricDensityStatistics.cpp
  GlobalImageFeatures/mitkGIFIntensityVolumeHistogramFeatures.cpp
  GlobalImageFeatures/mitkGIFNeighbourhoodGreyToneDifferenceFeatures.cpp
  GlobalImageFeatures/mitkGIFCurvatureStatistic.cpp

  MiniAppUtils/mitkGlobalImageFeaturesParameter.cpp
  MiniAppUtils/mitkSplitParameterToVector.cpp

  mitkCLUtil.cpp

)

set( TOOL_FILES
)
