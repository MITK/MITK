IF(BUILD_TESTING)
 INCLUDE(MITKIGTTrackingDeviceHardwareTesting.cmake)
ENDIF(BUILD_TESTING)

MITK_CREATE_MODULE_TESTS()

IF(MITK_NDI_AURORA_CONNECTED)
  mitkAddCustomModuleTest(mitkNDIAuroraHardwareTest mitkNDIAuroraHardwareTest ${MITK_NDI_AURORA_COM_PORT})
ENDIF(MITK_NDI_AURORA_CONNECTED)

IF(MITK_NDI_POLARIS_CONNECTED)
  mitkAddCustomModuleTest(mitkNDIPolarisHardwareTest mitkNDIPolarisHardwareTest ${MITK_NDI_POLARIS_COM_PORT})
ENDIF(MITK_NDI_POLARIS_CONNECTED)

IF(MITK_CLARON_MICRONTRACKER_CONNECTED)
  mitkAddCustomModuleTest(mitkClaronTrackingDeviceHardwareTest  mitkClaronTrackingDeviceHardwareTest ${MITK_DATA_DIR}/IGT-Data/MicronTrackerToolfiles/Tool_11 ${MITK_DATA_DIR}/IGT-Data/MicronTrackerToolfiles/Tool_12 ${MITK_DATA_DIR}/IGT-Data/MicronTrackerToolfiles/Tool_13)
ENDIF(MITK_CLARON_MICRONTRACKER_CONNECTED)
