set(MODULE_TESTS
  mitkPlanarCrossTest.cpp
  mitkPlanarPolygonTest.cpp
  mitkPlanarSubdivisionPolygonTest.cpp
  mitkPlanarFigureIOTest.cpp
  mitkPlanarFigureObjectFactoryTest.cpp
  mitkPlanarArrowTest.cpp
)

set(MODULE_CUSTOM_TESTS
  mitkViewportRenderingTest.cpp
)

if(MITK_ENABLE_RENDERING_TESTING) #since mitkInteractionTestHelper is currently creating a vtkRenderWindow
set(MODULE_TESTS
  ${MODULE_TESTS}
  mitkPlanarFigureInteractionTest.cpp
)
endif()
