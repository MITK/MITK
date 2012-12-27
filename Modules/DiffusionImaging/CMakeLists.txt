find_package(ITK)
if(ITK_GDCM_DIR)
  include(${ITK_GDCM_DIR}/GDCMConfig.cmake)
  if(GDCM_MAJOR_VERSION EQUAL 2)
    add_definitions(-DGDCM2)
    set(ITK_USES_GDCM2 1)
  endif(GDCM_MAJOR_VERSION EQUAL 2)
endif(ITK_GDCM_DIR)

MITK_CREATE_MODULE( MitkDiffusionImaging
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS SignalModels IODataStructures Reconstruction Tractography Tractography/GibbsTracking Rendering Algorithms DicomImport Interactions IODataStructures/DiffusionWeightedImages IODataStructures/QBallImages IODataStructures/TensorImages IODataStructures/FiberBundle IODataStructures/FiberBundleX IODataStructures/PlanarFigureComposite IODataStructures/TbssImages Algorithms/Connectomics IODataStructures/ConnectomicsNetwork  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkExt SceneSerializationBase QmitkExt MitkGraphAlgorithms
  PACKAGE_DEPENDS Boost
)

MITK_USE_MODULE(MitkDiffusionImaging)
if(MitkDiffusionImaging_IS_ENABLED)
  file(DOWNLOAD http://mitk.org/download/data/FibertrackingLUT.tar.gz ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/FibertrackingLUT.tar.gz TIMEOUT 10)
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} tar xzf FibertrackingLUT.tar.gz)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Rendering/mitkShaderFiberClipping.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mitkShaderFiberClipping.xml)
  MITK_INSTALL(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mitkShaderFiberClipping.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/FiberTrackingLUTBaryCoords.bin ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/FiberTrackingLUTIndices.bin)
endif()

add_subdirectory(Testing)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mitkDiffusionImagingConfigure.h.in ${CMAKE_CURRENT_BINARY_DIR}/mitkDiffusionImagingConfigure.h)

