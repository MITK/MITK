file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkImageStatisticsCalculator.cpp
  mitkImageStatisticsContainer.cpp
  mitkPointSetStatisticsCalculator.cpp
  mitkPointSetDifferenceStatisticsCalculator.cpp
  mitkIntensityProfile.cpp
  #See T30375
  #mitkHotspotMaskGenerator.cpp
  mitkMaskGenerator.cpp
  mitkPlanarFigureMaskGenerator.cpp
  mitkMultiLabelMaskGenerator.cpp
  mitkImageMaskGenerator.cpp
  mitkHistogramStatisticsCalculator.cpp
  mitkIgnorePixelMaskGenerator.cpp
  mitkImageStatisticsPredicateHelper.cpp
  mitkImageStatisticsContainerNodeHelper.cpp
  mitkImageStatisticsContainerManager.cpp
  mitkStatisticsToImageRelationRule.cpp
  mitkStatisticsToMaskRelationRule.cpp
  mitkImageStatisticsConstants.cpp
)
