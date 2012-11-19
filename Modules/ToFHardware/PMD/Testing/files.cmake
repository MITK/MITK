set(MODULE_TESTS
    mitkToFCameraPMDPlayerControllerTest.cpp
    mitkToFCameraPMDPlayerDeviceTest.cpp
    mitkToFCameraPMDControllerTest.cpp
    mitkToFCameraPMDDeviceTest.cpp
    mitkToFCameraPMDRawDataDeviceTest.cpp
)

IF(MITK_USE_TOF_PMDCAMCUBE)
SET(MODULE_TESTS ${MODULE_TESTS}
    mitkToFCameraPMDCamCubeControllerTest.cpp
    mitkToFCameraPMDCamCubeDeviceTest.cpp
    mitkToFCameraPMDRawDataCamCubeDeviceTest.cpp
)
ENDIF(MITK_USE_TOF_PMDCAMCUBE)

IF(MITK_USE_TOF_PMDO3)
SET(MODULE_TESTS ${MODULE_TESTS}
    mitkToFCameraPMDO3ControllerTest.cpp
    mitkToFCameraPMDO3DeviceTest.cpp
)
ENDIF(MITK_USE_TOF_PMDO3)


IF(MITK_USE_TOF_PMDCAMBOARD)
SET(MODULE_TESTS ${MODULE_TESTS}
    mitkToFCameraPMDCamBoardControllerTest.cpp
    mitkToFCameraPMDCamBoardDeviceTest.cpp
    mitkToFCameraPMDRawDataCamBoardDeviceTest.cpp
    )
ENDIF(MITK_USE_TOF_PMDCAMBOARD)
