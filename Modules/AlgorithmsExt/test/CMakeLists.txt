MITK_CREATE_MODULE_TESTS()
if(TARGET ${TESTDRIVER})
  mitkAddCustomModuleTest(mitkLabeledImageToSurfaceFilterTest_BinaryBall  mitkLabeledImageToSurfaceFilterTest ${MITK_DATA_DIR}/BallBinary30x30x30.pic.gz)
endif()

