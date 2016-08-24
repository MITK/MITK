file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkAnnotationPlacer.cpp
  mitkColorBarOverlay.cpp
  mitkLabelOverlay3D.cpp
  mitkLogoOverlay.cpp
  mitkOverlayLayouter2D.cpp
  mitkScaleLegendOverlay.cpp
  mitkTextOverlay2D.cpp
  mitkTextOverlay3D.cpp
  mitkVtkLogoRepresentation.cxx
  mitkVtkOverlay.cpp
  mitkVtkOverlay2D.cpp
  mitkVtkOverlay3D.cpp
)

