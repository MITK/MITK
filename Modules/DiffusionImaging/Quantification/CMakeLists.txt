#DiffusionImaging/Quantification
MITK_CREATE_MODULE( Quantification
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms IODataStructures IODataStructures/TbssImages Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS DiffusionCore FiberTracking MitkGraphAlgorithms QmitkExt
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)

