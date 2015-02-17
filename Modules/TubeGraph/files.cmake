file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  DataStructure/mitkCircularProfileTubeElement.cpp
  DataStructure/mitkTubeGraph.cpp
  DataStructure/mitkTubeGraphEdge.cpp
  DataStructure/mitkTubeGraphVertex.cpp
  Interactions/mitkTubeGraphDataInteractor.cpp
  Interactions/mitkTubeGraphPicker.cpp
  IO/mitkTubeGraphDefinitions.cpp
  IO/mitkTubeGraphIO.cpp
  IO/mitkTubeGraphModuleActivator.cpp
  IO/mitkTubeGraphObjectFactory.cpp
  Rendering/mitkTubeGraphProperty.cpp
  Rendering/mitkTubeGraphVtkMapper3D.cpp
)

set(RESOURCE_FILES
 Interactions/TubeGraphConfig.xml
 Interactions/TubeGraphInteraction.xml
)
