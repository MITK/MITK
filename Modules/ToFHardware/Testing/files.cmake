set(MODULE_TESTS
  #mitkThreadedToFRawDataReconstructionTest.cpp
  mitkAbstractToFDeviceFactoryTest.cpp
  mitkToFCameraMITKPlayerDeviceTest.cpp
  mitkToFCameraMITKPlayerDeviceFactoryTest.cpp
  mitkToFImageCsvWriterTest.cpp
  mitkToFImageGrabberTest.cpp
  #mitkToFImageRecorderTest.cpp
  #mitkToFImageRecorderFilterTest.cpp
  mitkToFImageWriterTest.cpp
  mitkToFNrrdImageWriterTest.cpp
  mitkToFOpenCVImageGrabberTest.cpp
)

set(MODULE_CUSTOM_TESTS
  mitkPlayerLoadAndRenderDepthDataTest.cpp
  mitkPlayerLoadAndRenderRGBDataTest.cpp
)

# Create an artificial module initializing class for
# the mitkToFCameraMITKPlayerDeviceFactoryTest
usFunctionGenerateExecutableInit(testdriver_init_file
                                 IDENTIFIER ${MODULE_NAME}TestDriver
                                )

set(TEST_CPP_FILES ${testdriver_init_file})
