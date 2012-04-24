if(BUILD_TESTING)
 include(MITKIGTTrackingDeviceHardwareTesting.cmake)
endif(BUILD_TESTING)

MITK_CREATE_MODULE_TESTS()

if(MITK_NDI_AURORA_CONNECTED)
  mitkAddCustomModuleTest(mitkNDIAuroraHardwareTest mitkNDIAuroraHardwareTest ${MITK_NDI_AURORA_COM_PORT})
endif(MITK_NDI_AURORA_CONNECTED)

if(MITK_NDI_POLARIS_CONNECTED)
  mitkAddCustomModuleTest(mitkNDIPolarisHardwareTest mitkNDIPolarisHardwareTest ${MITK_NDI_POLARIS_COM_PORT})
endif(MITK_NDI_POLARIS_CONNECTED)

if(MITK_CLARON_MICRONTRACKER_CONNECTED)
  mitkAddCustomModuleTest(mitkClaronTrackingDeviceHardwareTest  mitkClaronTrackingDeviceHardwareTest ${MITK_DATA_DIR}/IGT-Data/MicronTrackerToolfiles/Tool_11 ${MITK_DATA_DIR}/IGT-Data/MicronTrackerToolfiles/Tool_12 ${MITK_DATA_DIR}/IGT-Data/MicronTrackerToolfiles/Tool_13)
endif(MITK_CLARON_MICRONTRACKER_CONNECTED)
