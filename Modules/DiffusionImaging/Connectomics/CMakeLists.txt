MITK_CREATE_MODULE( Connectomics
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms IODataStructures Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS DiffusionCore FiberTracking QmitkExt
  PACKAGE_DEPENDS VTK|vtkInfovisLayout
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
