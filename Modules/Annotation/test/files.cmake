
set(MODULE_TESTS
    mitkAnnotationTest.cpp
)

if(MITK_ENABLE_RENDERING_TESTING)
set(MODULE_TESTS
  ${MODULE_TESTS}
  mitkManualPlacementAnnotationRendererTest.cpp
  mitkColorBarAnnotationTest.cpp
  mitkLabelAnnotation3DTest.cpp
  mitkLogoAnnotationTest.cpp
  mitkLayoutAnnotationRendererTest.cpp
  mitkScaleLegendAnnotationTest.cpp
  mitkTextAnnotation2DTest.cpp
  mitkTextAnnotation3DTest.cpp
)
endif()
