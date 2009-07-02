SET(MODULE_TESTS 
  # itkGraphCutSegmentationFilterTest.cpp
  # itkImageToTreeFilterTest.cpp
  # itkThreadPoolTest.cpp
  mitkAutoCropImageFilterTest.cpp
  # mitkClosedSplineTest.cpp
  mitkContourMapper2DTest.cpp
  mitkContourTest.cpp
  # mitkDataTreeFilterTest.cpp
  # mitkDataTreeTest.cpp
  mitkDemonsRegistrationTest.cpp
  mitkExternalToolsTest.cpp
  # mitkGeometryTest.cpp
  mitkHistogramMatchingTest.cpp
  # mitkLabeledImageToSurfaceFilterTest.cpp
  mitkMeshTest.cpp
  mitkOrganTypePropertyTest.cpp
  mitkPipelineSmartPointerCorrectnessTest.cpp
  mitkPlaneFitTest.cpp
  mitkPointLocatorTest.cpp
  # mitkReportGeneratorTest.cpp
  # mitkRequestedRegionTest.cpp
  # mitkRigidRegistrationPresetTest.cpp
  # mitkRingSplineTest.cpp
  # mitkSegmentationInterpolationTest.cpp
  mitkSymmetricForcesDemonsRegistrationTest.cpp
  # mitkTestTemplate.cpp
  # mitkToolManagerTest.cpp
  # vtkPolyDataToPovRayMesh2Test.cpp
)
SET(MODULE_IMAGE_TESTS
  mitkUnstructuredGridVtkWriterTest.cpp
  mitkCompressedImageContainerTest.cpp  
  mitkCylindricToCartesianFilterTest.cpp
  # failing test
  #mitkExtractImageFilterTest.cpp  
  mitkManualSegmentationToSurfaceFilterTest.cpp
  mitkOverwriteSliceImageFilterTest.cpp  
  mitkSurfaceToImageFilterTest.cpp
)
SET(MODULE_TESTIMAGES
  US4DCyl.pic.gz
  Pic3D.pic.gz
  Pic2DplusT.pic.gz
  BallBinary30x30x30.pic.gz
  Png2D-bw.png
  binary.stl
  ball.stl
)
