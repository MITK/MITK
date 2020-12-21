MITK_CREATE_MODULE_TESTS()
if(TARGET ${TESTDRIVER})
  mitkAddCustomModuleTest(mitkKinectReconstructionTest_LegoPhantom mitkKinectReconstructionTest
                          ${MITK_DATA_DIR}/ToF-Data/CalibrationFiles/Kinect_RGB_camera.xml #camera intrinsics
                          ${MITK_DATA_DIR}/ToF-Data/Kinect_Lego_Phantom_DistanceImage.nrrd #kinect distance image
  )

  #mitkAddCustomModuleTest(mitkToFImageDownsamplingFilterTest_20 mitkToFImageDownsamplingFilterTest PMDCamCube2_MF0_IT0_20Images_DistanceImage.pic)
  #mitkAddCustomModuleTest(mitkToFImageDownsamplingFilterTest_1 mitkToFImageDownsamplingFilterTest PMDCamCube2_MF0_IT0_1Images_DistanceImage.pic)
endif()
