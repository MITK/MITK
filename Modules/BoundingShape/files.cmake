file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  DataManagement/mitkBoundingShapeUtil.cpp
  DataManagement/mitkBoundingShapeCropper.cpp
  DataManagement/mitkBoundingShapeObjectFactory.cpp
  Interactions/mitkBoundingShapeInteractor.cpp
  Rendering/mitkBoundingShapeVtkMapper2D.cpp
  Rendering/mitkBoundingShapeVtkMapper3D.cpp
)

set(RESOURCE_FILES
  Interactions/BoundingShapeMouseConfig.xml
  Interactions/BoundingShapeInteraction.xml
)
