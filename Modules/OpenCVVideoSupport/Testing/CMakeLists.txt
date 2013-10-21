MITK_CREATE_MODULE_TESTS()

IF(BUILD_TESTING AND MODULE_IS_ENABLED)
  mitkAddCustomModuleTest("mitkOpenCVMitkConversionTest" "mitkOpenCVMitkConversionTest"
  "${MITK_DATA_DIR}/ToF-Data/Kinect_LiverPhantom_DistanceImage.nrrd"
  "${MITK_DATA_DIR}/ToF-Data/Kinect_LiverPhantom_RGBImage.nrrd"
  "${MITK_DATA_DIR}/Png2D-bw.png"
  "${MITK_DATA_DIR}/NrrdWritingTestImage.jpg"
  )
ENDIF(BUILD_TESTING AND MODULE_IS_ENABLED)