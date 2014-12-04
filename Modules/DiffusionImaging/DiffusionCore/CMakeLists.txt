# With apple gcc 4.2.1 the following waring leads to an build error if boost is enabled
if(APPLE)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=empty-body" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms Algorithms/Reconstruction Algorithms/Registration Algorithms/Reconstruction/MultishellProcessing DicomImport IODataStructures/DiffusionWeightedImages IODataStructures/QBallImages IODataStructures/TensorImages IODataStructures Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkMapperExt MitkPlanarFigure MitkImageExtraction MitkSceneSerializationBase MitkDICOMReader
  PACKAGE_DEPENDS VTK|vtkFiltersProgrammable ITK|ITKDistanceMap+ITKRegistrationCommon+ITKLabelVoting+ITKVTK Boost
                  ITK|ITKMetricsv4+ITKRegistrationMethodsv4
  WARNINGS_AS_ERRORS
)

if(MSVC)
  mitkFunctionCheckCAndCXXCompilerFlags("/wd4005" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

add_subdirectory(Testing)
