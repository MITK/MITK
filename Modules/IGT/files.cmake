set(CPP_FILES
  IGTFilters/mitkNavigationDataLandmarkTransformFilter.cpp
  IGTFilters/mitkNavigationDataReferenceTransformFilter.cpp
  IGTFilters/mitkNavigationDataTransformFilter.cpp
  IGTFilters/mitkNavigationDataRecorder.cpp
  IGTFilters/mitkNavigationDataPlayer.cpp
  IGTFilters/mitkNavigationDataPlayerBase.cpp
  IGTFilters/mitkNavigationDataObjectVisualizationFilter.cpp
  IGTFilters/mitkCameraVisualization.cpp
  IGTFilters/mitkNavigationData.cpp
  IGTFilters/mitkNavigationDataDisplacementFilter.cpp
  IGTFilters/mitkNavigationDataSequentialPlayer.cpp
  IGTFilters/mitkNavigationDataSource.cpp
  IGTFilters/mitkNavigationDataToMessageFilter.cpp
  IGTFilters/mitkNavigationDataToNavigationDataFilter.cpp
  IGTFilters/mitkNavigationDataToPointSetFilter.cpp
  IGTFilters/mitkNavigationDataEvaluationFilter.cpp

  IGTFilters/mitkTrackingDeviceSource.cpp
  IGTFilters/mitkTrackingVolumeGenerator.cpp
  IGTFilters/mitkTimeStamp.cpp
  IGTFilters/mitkRealTimeClock.cpp
  IGTFilters/mitkTrackingDeviceSourceConfigurator.cpp

  IGTTrackingDevices/mitkClaronTool.cpp
  IGTTrackingDevices/mitkClaronTrackingDevice.cpp
  IGTTrackingDevices/mitkInternalTrackingTool.cpp
  IGTTrackingDevices/mitkNDIPassiveTool.cpp
  IGTTrackingDevices/mitkNDIProtocol.cpp
  IGTTrackingDevices/mitkNDITrackingDevice.cpp
  IGTTrackingDevices/mitkSerialCommunication.cpp
  IGTTrackingDevices/mitkTrackingDevice.cpp
  IGTTrackingDevices/mitkTrackingTool.cpp
  IGTTrackingDevices/mitkVirtualTrackingDevice.cpp
  IGTTrackingDevices/mitkVirtualTrackingTool.cpp

  IGTToolManagement/mitkNavigationToolStorage.cpp
  IGTToolManagement/mitkNavigationToolStorageSerializer.cpp
  IGTToolManagement/mitkNavigationToolStorageDeserializer.cpp
  IGTToolManagement/mitkNavigationTool.cpp
  IGTToolManagement/mitkNavigationToolReader.cpp
  IGTToolManagement/mitkNavigationToolWriter.cpp

  IGTExceptionHandling/mitkIGTException.cpp
  IGTExceptionHandling/mitkIGTHardwareException.cpp
  IGTExceptionHandling/mitkIGTIOException.cpp
)

if(MITK_USE_MICRON_TRACKER)
  set(CPP_FILES ${CPP_FILES} IGTTrackingDevices/mitkClaronInterface.cpp)
else()
  set(CPP_FILES ${CPP_FILES} IGTTrackingDevices/mitkClaronInterfaceStub.cpp)
endif(MITK_USE_MICRON_TRACKER)

if(MITK_USE_MICROBIRD_TRACKER)
  set(CPP_FILES ${CPP_FILES} IGTTrackingDevices/mitkMicroBirdTrackingDevice.cpp)
endif(MITK_USE_MICROBIRD_TRACKER)

if(WIN32)
  set(CPP_FILES ${CPP_FILES} IGTFilters/mitkWindowsRealTimeClock.cpp)
else()
  set(CPP_FILES ${CPP_FILES} IGTFilters/mitkLinuxRealTimeClock.cpp)
endif(WIN32)
