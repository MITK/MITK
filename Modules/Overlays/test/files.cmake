
set(MODULE_TESTS
    mitkOverlaysTest.cpp
)

if(MITK_ENABLE_RENDERING_TESTING)
set(MODULE_TESTS
  ${MODULE_TESTS}
  mitkAnnotationPlacerTest.cpp
  mitkColorBarOverlayTest.cpp
  mitkLabelOverlay3DTest.cpp
  mitkLogoOverlayTest.cpp
  mitkOverlayLayouter2DTest.cpp
  mitkScaleLegendOverlayTest.cpp
  mitkTextOverlay2DTest.cpp
  mitkTextOverlay3DTest.cpp
)
endif()
