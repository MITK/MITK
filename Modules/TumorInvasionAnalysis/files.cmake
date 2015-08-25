file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

SET(CPP_FILES
  Classification/mitkTumorInvasionClassification.cpp
  ImageFilters/mitkCollectionDilatation.cpp
  ImageFilters/mitkCollectionGrayOpening.cpp
)

