file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  Algorithms/itkLabelSampler.cpp
  Algorithms/itkSmoothedClassProbabilites.cpp

  Features/itkNeighborhoodFunctorImageFilter.cpp

  GlobalImageFeatures/mitkGIFCooccurenceMatrix.cpp
  mitkCLUtil.cpp

)

set( TOOL_FILES
)
