file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkControlPointManager.cpp
  mitkDICOMHelper.cpp
  mitkNodePredicates.cpp
  mitkRelationStorage.cpp
  mitkSemanticRelations.cpp
  mitkUIDGeneratorBoost.cpp
)
