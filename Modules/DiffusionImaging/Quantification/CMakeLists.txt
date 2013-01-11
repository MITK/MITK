#DiffusionImaging/Quantification
MITK_CREATE_MODULE( Quantification
  INCLUDE_DIRS Algorithms IODataStructures IODataStructures/TbssImages Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS DiffusionCore FiberTracking MitkGraphAlgorithms
)

add_subdirectory(Testing)

