file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkManualPlacementAnnotationRenderer.cpp
  mitkColorBarAnnotation.cpp
  mitkLabelAnnotation3D.cpp
  mitkLogoAnnotation.cpp
  mitkLayoutAnnotationRenderer.cpp
  mitkScaleLegendAnnotation.cpp
  mitkTextAnnotation2D.cpp
  mitkTextAnnotation3D.cpp
  mitkVtkLogoRepresentation.cxx
  mitkVtkAnnotation.cpp
  mitkVtkAnnotation2D.cpp
  mitkVtkAnnotation3D.cpp
)

