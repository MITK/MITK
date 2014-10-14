# With apple gcc 4.2.1 the following waring leads to an build error if boost is enabled
if(APPLE)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=empty-body" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms Algorithms/Reconstruction Algorithms/Registration Algorithms/Reconstruction/MultishellProcessing DicomImport IODataStructures/DiffusionWeightedImages IODataStructures/QBallImages IODataStructures/TensorImages IODataStructures Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkMapperExt MitkPlanarFigure MitkImageExtraction MitkSceneSerializationBase
  PACKAGE_DEPENDS VTK|vtkFiltersProgrammable ITK|ITKDistanceMap+ITKRegistrationCommon+ITKLabelVoting+ITKVTK Boost
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
