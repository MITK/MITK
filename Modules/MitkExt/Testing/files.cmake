set(MODULE_TESTS
  mitkAutoCropImageFilterTest.cpp
  mitkBoundingObjectCutterTest.cpp
  mitkCoreExtObjectFactoryTest
  mitkDataNodeExtTest.cpp
  mitkExternalToolsTest.cpp
  mitkMeshTest.cpp
  mitkMultiStepperTest.cpp
  mitkOrganTypePropertyTest.cpp
  # mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPlaneFitTest.cpp
  mitkPointLocatorTest.cpp
  mitkUnstructuredGridTest.cpp
  mitkSimpleHistogramTest.cpp
)
set(MODULE_IMAGE_TESTS
  mitkUnstructuredGridVtkWriterTest.cpp #crappy test, but it can run with surfaces and images
  mitkCompressedImageContainerTest.cpp #only runs on images
  mitkCylindricToCartesianFilterTest.cpp #only runs on images
)
set(MODULE_SURFACE_TESTS
  mitkSurfaceToImageFilterTest.cpp #only runs on surfaces
  mitkUnstructuredGridVtkWriterTest #crappy test, but it can run with surfaces and images
)
set(MODULE_CUSTOM_TESTS
  mitkLabeledImageToSurfaceFilterTest.cpp
  mitkImageToSurfaceFilterTest.cpp
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
