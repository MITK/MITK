MITK_CREATE_MODULE_TESTS()
if(TARGET ${TESTDRIVER})
  mitk_use_modules(TARGET ${TESTDRIVER} PACKAGES VTK|vtkTestingRendering)

  #Only enabled for Windows and Mac nightlies.
  #If you want to run this on linux, just enable the flag in the superbuild.
  if(MITK_ENABLE_RENDERING_TESTING)

  mitkAddCustomModuleTest(mitkPlayerLoadAndRenderDepthDataTest_KinectDepthImage #testname
    mitkPlayerLoadAndRenderDepthDataTest #testclassname
    Kinect_LiverPhantom_DistanceImage.nrrd #input image
    -V ${MITK_DATA_DIR}/ToF-Data/ReferenceScreenshots/Kinect_LiverPhantom_DistanceImage640x480REF.png #reference image
  )

  mitkAddCustomModuleTest(mitkPlayerLoadAndRenderRGBDataTest_KinectRGBImage mitkPlayerLoadAndRenderRGBDataTest Kinect_LiverPhantom_RGBImage.nrrd -V ${MITK_DATA_DIR}/ToF-Data/ReferenceScreenshots/Kinect_LiverPhantom_RGBImage640x480REF.png)

  #rendering tests cannot run in parallel
  SET_PROPERTY(TEST mitkPlayerLoadAndRenderDepthDataTest_KinectDepthImage mitkPlayerLoadAndRenderRGBDataTest_KinectRGBImage PROPERTY RUN_SERIAL TRUE)

  endif()
endif()

