MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms Algorithms/BrainParcellation IODataStructures Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkDiffusionCore MitkFiberTracking
  PACKAGE_DEPENDS VTK|vtkInfovisLayout
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
