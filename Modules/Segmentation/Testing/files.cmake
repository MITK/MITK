set(MODULE_TESTS
  mitkContourMapper2DTest.cpp
  mitkContourTest.cpp
  mitkContourModelSetToImageFilterTest.cpp
  mitkDataNodeSegmentationTest.cpp
  mitkImageToContourFilterTest.cpp
#  mitkSegmentationInterpolationTest.cpp
  mitkOverwriteSliceFilterTest.cpp
  mitkOverwriteSliceFilterObliquePlaneTest.cpp
#  mitkToolManagerTest.cpp
  mitkToolManagerProviderTest.cpp
  mitkManualSegmentationToSurfaceFilterTest.cpp #new cpp unit style
)

if(MITK_ENABLE_RENDERING_TESTING) #since mitkInteractionTestHelper is currently creating a vtkRenderWindow
set(MODULE_TESTS
  ${MODULE_TESTS}
  mitkToolInteractionTest.cpp
)
endif()

set(MODULE_IMAGE_TESTS
  mitkOverwriteSliceImageFilterTest.cpp #only runs on images
)
set(MODULE_CUSTOM_TESTS
)

set(MODULE_TESTIMAGES
  US4DCyl.nrrd
  Pic3D.nrrd
  Pic2DplusT.nrrd
  BallBinary30x30x30.nrrd
  Png2D-bw.png
)
