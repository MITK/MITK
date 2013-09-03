set(CPP_FILES
  Algorithms/mitkNavigationDataDisplacementFilter.cpp
  Algorithms/mitkNavigationDataEvaluationFilter.cpp
  Algorithms/mitkNavigationDataLandmarkTransformFilter.cpp
  Algorithms/mitkNavigationDataReferenceTransformFilter.cpp
  Algorithms/mitkNavigationDataToMessageFilter.cpp
  Algorithms/mitkNavigationDataToNavigationDataFilter.cpp
  Algorithms/mitkNavigationDataToPointSetFilter.cpp
  Algorithms/mitkNavigationDataTransformFilter.cpp

  Common/mitkIGTTimeStamp.cpp
  Common/mitkRealTimeClock.cpp
  Common/mitkSerialCommunication.cpp

  DataManagement/mitkNavigationData.cpp
  DataManagement/mitkNavigationDataSet.cpp
  DataManagement/mitkNavigationDataSource.cpp
  DataManagement/mitkNavigationTool.cpp
  DataManagement/mitkNavigationToolStorage.cpp
  DataManagement/mitkTrackingDeviceSourceConfigurator.cpp
  DataManagement/mitkTrackingDeviceSource.cpp

  ExceptionHandling/mitkIGTException.cpp
  ExceptionHandling/mitkIGTHardwareException.cpp
  ExceptionHandling/mitkIGTIOException.cpp

  IO/mitkNavigationDataPlayer.cpp
  IO/mitkNavigationDataPlayerBase.cpp
  IO/mitkNavigationDataRecorder.cpp
  IO/mitkNavigationDataSequentialPlayer.cpp
  IO/mitkNavigationToolReader.cpp
  IO/mitkNavigationToolStorageSerializer.cpp
  IO/mitkNavigationToolStorageDeserializer.cpp
  IO/mitkNavigationToolWriter.cpp
  IO/mitkNavigationDataReaderInterface.cpp
  IO/mitkNavigationDataReaderXML.cpp
  IO/mitkNavigationDataReaderCSV.cpp

  Rendering/mitkCameraVisualization.cpp
  Rendering/mitkNavigationDataObjectVisualizationFilter.cpp

  TrackingDevices/mitkClaronTool.cpp
  TrackingDevices/mitkClaronTrackingDevice.cpp
  TrackingDevices/mitkInternalTrackingTool.cpp
  TrackingDevices/mitkNDIPassiveTool.cpp
  TrackingDevices/mitkNDIProtocol.cpp
  TrackingDevices/mitkNDITrackingDevice.cpp
  TrackingDevices/mitkTrackingDevice.cpp
  TrackingDevices/mitkTrackingTool.cpp
  TrackingDevices/mitkTrackingVolumeGenerator.cpp
  TrackingDevices/mitkVirtualTrackingDevice.cpp
  TrackingDevices/mitkVirtualTrackingTool.cpp
)

if(MITK_USE_MICRON_TRACKER)
  set(CPP_FILES ${CPP_FILES} TrackingDevices/mitkClaronInterface.cpp)
else()
  set(CPP_FILES ${CPP_FILES} TrackingDevices/mitkClaronInterfaceStub.cpp)
endif(MITK_USE_MICRON_TRACKER)

if(MITK_USE_MICROBIRD_TRACKER)
  set(CPP_FILES ${CPP_FILES} TrackingDevices/mitkMicroBirdTrackingDevice.cpp)
endif(MITK_USE_MICROBIRD_TRACKER)

if(WIN32)
  set(CPP_FILES ${CPP_FILES} Common/mitkWindowsRealTimeClock.cpp)
else()
  set(CPP_FILES ${CPP_FILES} Common/mitkLinuxRealTimeClock.cpp)
endif(WIN32)
