set(MODULE_TESTS
  mitkContourMapper2DTest.cpp
  mitkContourTest.cpp
  mitkDataNodeSegmentationTest.cpp
#  mitkSegmentationInterpolationTest.cpp
  mitkOverwriteSliceFilterTest.cpp
#  mitkOverwriteSliceFilterObliquePlaneTest.cpp
  #mitkContourModelTest.cpp
  #mitkContourModelIOTest.cpp
#  mitkToolManagerTest.cpp
  mitkToolManagerProviderTest.cpp
)

set(MODULE_IMAGE_TESTS
  mitkManualSegmentationToSurfaceFilterTest.cpp #only runs on images
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

# Create an artificial module initializing class for
# usServices
set(testdriver_init_file )
usFunctionGenerateModuleInit(testdriver_init_file
                             NAME SegmentationTestDriver
                             DEPENDS "Segmentation"
                             VERSION "0.1.0"
                            )

set(TEST_CPP_FILES ${testdriver_init_file})
