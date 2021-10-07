set(MODULE_TESTS
  mitkContourMapper2DTest.cpp
  mitkContourTest.cpp
  mitkContourModelSetToImageFilterTest.cpp
  mitkDataNodeSegmentationTest.cpp
  mitkFeatureBasedEdgeDetectionFilterTest.cpp
  mitkImageToContourFilterTest.cpp
  mitkSegmentationInterpolationTest.cpp
  mitkOverwriteSliceFilterTest.cpp
  mitkOverwriteSliceFilterObliquePlaneTest.cpp
#  mitkToolManagerTest.cpp
  mitkToolManagerProviderTest.cpp
  mitkManualSegmentationToSurfaceFilterTest.cpp #new cpp unit style
  mitkToolInteractionTest.cpp
)

set(MODULE_CUSTOM_TESTS
)

set(MODULE_TESTIMAGE
  US4DCyl.nrrd
  Pic3D.nrrd
  Pic2DplusT.nrrd
  BallBinary30x30x30.nrrd
  Png2D-bw.png
)
