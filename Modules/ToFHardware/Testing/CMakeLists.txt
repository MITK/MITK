MITK_CREATE_MODULE_TESTS()
if(TARGET ${TESTDRIVER})
  mitk_use_modules(TARGET ${TESTDRIVER} PACKAGES VTK|TestingRendering)

  mitkAddCustomModuleRenderingTest(mitkPlayerLoadAndRenderDepthDataTest_KinectDepthImage #testname
    mitkPlayerLoadAndRenderDepthDataTest #testclassname
    Kinect_LiverPhantom_DistanceImage.nrrd #input image
    -V ${MITK_DATA_DIR}/ToF-Data/ReferenceScreenshots/Kinect_LiverPhantom_DistanceImage640x480REF.png #reference image
  )

  mitkAddCustomModuleRenderingTest(mitkPlayerLoadAndRenderRGBDataTest_KinectRGBImage mitkPlayerLoadAndRenderRGBDataTest Kinect_LiverPhantom_RGBImage.nrrd -V ${MITK_DATA_DIR}/ToF-Data/ReferenceScreenshots/Kinect_LiverPhantom_RGBImage640x480REF.png)
endif()
