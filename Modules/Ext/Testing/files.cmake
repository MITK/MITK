set(MODULE_TESTS
  mitkDataNodeExtTest.cpp
  mitkExternalToolsTest.cpp
  # mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPlaneFitTest.cpp
  mitkPointLocatorTest.cpp
)
set(MODULE_IMAGE_TESTS
  mitkCylindricToCartesianFilterTest.cpp #only runs on images
)

set(MODULE_TESTIMAGES
  US4DCyl.nrrd
  Pic3D.nrrd
  Pic2DplusT.nrrd
  BallBinary30x30x30.nrrd
  Png2D-bw.png
)
set(MODULE_TESTSURFACES
  binary.stl
  ball.stl
)
