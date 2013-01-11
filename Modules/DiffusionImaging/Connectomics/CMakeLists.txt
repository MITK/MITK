MITK_CREATE_MODULE( Connectomics
  INCLUDE_DIRS Algorithms IODataStructures Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS DiffusionCore FiberTracking
)

add_subdirectory(Testing)