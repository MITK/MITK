SET(CPP_FILES
  IGTFilters/mitkNavigationDataLandmarkTransformFilter.cpp
  IGTFilters/mitkNavigationDataTransformFilter.cpp
  IGTFilters/mitkNavigationDataRecorder.cpp
  IGTFilters/mitkNavigationDataPlayer.cpp
  IGTFilters/mitkNavigationDataObjectVisualizationFilter.cpp
  IGTFilters/mitkCameraVisualization.cpp
  IGTFilters/mitkNavigationData.cpp
  IGTFilters/mitkNavigationDataDisplacementFilter.cpp
  IGTFilters/mitkNavigationDataSource.cpp
  IGTFilters/mitkNavigationDataToMessageFilter.cpp
  IGTFilters/mitkNavigationDataToNavigationDataFilter.cpp
  IGTFilters/mitkNavigationDataToPointSetFilter.cpp
  IGTFilters/mitkTrackingDeviceSource.cpp
  IGTFilters/mitkTimeStamp.cpp
  IGTFilters/mitkRealTimeClock.cpp

  IGTTrackingDevices/mitkClaronTool.cpp
  IGTTrackingDevices/mitkClaronTrackingDevice.cpp
  IGTTrackingDevices/mitkInternalTrackingTool.cpp
  IGTTrackingDevices/mitkNDIPassiveTool.cpp
  IGTTrackingDevices/mitkNDIProtocol.cpp
  IGTTrackingDevices/mitkNDITrackingDevice.cpp
  IGTTrackingDevices/mitkSerialCommunication.cpp
  IGTTrackingDevices/mitkTrackingDevice.cpp
  IGTTrackingDevices/mitkTrackingTool.cpp
  IGTTrackingDevices/mitkTrackingVolume.cpp
  IGTTrackingDevices/mitkRandomTrackingDevice.cpp
)

IF(MITK_USE_MICRON_TRACKER)
  SET(CPP_FILES ${CPP_FILES} IGTTrackingDevices/mitkClaronInterface.cpp)
ELSE()
  SET(CPP_FILES ${CPP_FILES} IGTTrackingDevices/mitkClaronInterfaceStub.cpp)
ENDIF(MITK_USE_MICRON_TRACKER)

IF(MITK_USE_MICROBIRD_TRACKER)
  SET(CPP_FILES ${CPP_FILES} IGTTrackingDevices/mitkMicroBirdTrackingDevice.cpp)
ENDIF(MITK_USE_MICROBIRD_TRACKER)

IF(WIN32) 
  SET(CPP_FILES ${CPP_FILES} IGTFilters/mitkWindowsRealTimeClock.cpp)
ELSE()
  SET(CPP_FILES ${CPP_FILES} IGTFilters/mitkLinuxRealTimeClock.cpp)
ENDIF(WIN32)

