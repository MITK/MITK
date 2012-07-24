set(MODULE_TESTS
  mitkContourMapper2DTest.cpp
  mitkContourTest.cpp
  mitkDataNodeSegmentationTest.cpp
#  mitkSegmentationInterpolationTest.cpp
  ContourModelReaderTest.cpp
  ContourModelTest.cpp
  ContourModelWriterTest.cpp
)

set(MODULE_IMAGE_TESTS
  mitkManualSegmentationToSurfaceFilterTest.cpp
  mitkOverwriteSliceImageFilterTest.cpp
#Deactivate due to Bug 12340. Should be fixed soon!
#  mitkOverwriteSliceFilterTest.cpp
)
set(MODULE_CUSTOM_TESTS
)
set(MODULE_TESTIMAGES
  US4DCyl.nrrd
  Pic3D.nrrd
  Pic2DplusT.nrrd
  BallBinary30x30x30.nrrd
  Png2D-bw.png
  binary.stl
  ball.stl
)
