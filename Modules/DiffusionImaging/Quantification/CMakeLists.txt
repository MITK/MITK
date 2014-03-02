#DiffusionImaging/Quantification
MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms IODataStructures IODataStructures/TbssImages Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkDiffusionCore MitkFiberTracking MitkGraphAlgorithms
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
