set(MODULE_TESTS
#IF(MITK_USE_TOF_PMDCAMCUBE)
    mitkToFCameraPMDCamCubeControllerTest.cpp
    mitkToFCameraPMDCamCubeDeviceTest.cpp

    mitkToFCameraPMDRawDataCamCubeDeviceTest.cpp
#ENDIF(MITK_USE_TOF_PMDCAMCUBE)

#IF(MITK_USE_TOF_PMDCAMBOARD)
    mitkToFCameraPMDCamBoardControllerTest.cpp
    mitkToFCameraPMDCamBoardDeviceTest.cpp

    mitkToFCameraPMDRawDataCamBoardDeviceTest.cpp
#ENDIF(MITK_USE_TOF_PMDCAMBOARD)

#IF(MITK_USE_TOF_PMDO3)
    mitkToFCameraPMDO3ControllerTest.cpp
    mitkToFCameraPMDO3DeviceTest.cpp
#ENDIF(MITK_USE_TOF_PMDO3)

    mitkToFCameraPMDPlayerControllerTest.cpp
    mitkToFCameraPMDPlayerDeviceTest.cpp

    mitkToFCameraPMDControllerTest.cpp
    mitkToFCameraPMDDeviceTest.cpp

  mitkToFCameraPMDRawDataDeviceTest.cpp

)
