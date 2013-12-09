MITK_CREATE_MODULE_TESTS()

IF(BUILD_TESTING AND MODULE_IS_ENABLED)
  mitkAddCustomModuleTest("mitkGrabCutOpenCVImageFilterTest" "mitkGrabCutOpenCVImageFilterTest"
  "${MITK_DATA_DIR}/OpenCV-Data/GrabCutInput.png"
  "${MITK_DATA_DIR}/OpenCV-Data/GrabCutMask.png"
  "${MITK_DATA_DIR}/OpenCV-Data/GrabCutOutput.png"
  )
ENDIF(BUILD_TESTING AND MODULE_IS_ENABLED)
