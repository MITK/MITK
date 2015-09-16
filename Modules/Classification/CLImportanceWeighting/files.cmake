file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkDistModels.cpp
  mitkLinkModels.cpp
  mitkLRDensityEstimation.cpp
  mitkGeneralizedLinearModel.cpp
  #Algorithms/itkLabelSampler.cpp
  #Algorithms/itkSmoothedClassProbabilites.cpp

  #Features/itkNeighborhoodFunctorImageFilter.cpp
  #mitkCLUtil.cpp
)

set( TOOL_FILES
)
