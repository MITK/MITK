MITK_CREATE_MODULE( FiberTracking
  INCLUDE_DIRS Algorithms Algorithms/GibbsTracking Algorithms/StochasticTracking IODataStructures IODataStructures/FiberBundleX IODataStructures/PlanarFigureComposite Interactions SignalModels Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS DiffusionCore MitkGraphAlgorithms
)

if(FiberTracking_IS_ENABLED)
  file(DOWNLOAD http://mitk.org/download/data/FibertrackingLUT.tar.gz ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/FibertrackingLUT.tar.gz TIMEOUT 10)
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} tar xzf FibertrackingLUT.tar.gz)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Rendering/mitkShaderFiberClipping.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mitkShaderFiberClipping.xml)
  MITK_INSTALL(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mitkShaderFiberClipping.xml ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/FiberTrackingLUTBaryCoords.bin ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/FiberTrackingLUTIndices.bin)
endif()

add_subdirectory(Testing)