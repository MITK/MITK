# With apple gcc 4.2.1 the following waring leads to an build error if boost is enabled
if(APPLE)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=empty-body" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS include/ include/Algorithms include/Algorithms/Reconstruction include/Algorithms/Registration include/Algorithms/Reconstruction/MultishellProcessing include/Algorithms/Reconstruction/FittingFunctions include/DicomImport include/IODataStructures/DiffusionWeightedImages include/IODataStructures/Properties include/IODataStructures/OdfImages include/IODataStructures/TensorImages include/IODataStructures include/Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkMapperExt MitkPlanarFigure MitkImageExtraction MitkSceneSerializationBase MitkDICOMReader MitkMatchPointRegistration
  PACKAGE_DEPENDS
    PUBLIC ITK|ITKTestKernel+ITKRegistrationCommon+ITKMetricsv4+ITKRegistrationMethodsv4+ITKDistanceMap+ITKLabelVoting+ITKVTK
    PUBLIC VTK|vtkFiltersProgrammable  OpenMP|OpenMP_CXX
)

if(MSVC)
  mitkFunctionCheckCAndCXXCompilerFlags("/wd4005" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

add_subdirectory(Testing)
add_subdirectory(autoload/IO)

if(MITK_USE_Python)
MITK_INSTALL(FILES PythonRequirements.txt)
endif()
