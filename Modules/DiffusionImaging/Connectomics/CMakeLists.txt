MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms Algorithms/BrainParcellation IODataStructures Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkDiffusionCore MitkFiberTracking
  PACKAGE_DEPENDS VTK|vtkInfovisLayout
)

add_subdirectory(Testing)
add_subdirectory(cmdapps)
