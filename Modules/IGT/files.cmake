set(CPP_FILES

  TestingHelper/mitkNavigationToolStorageTestHelper.cpp

  Algorithms/mitkNavigationDataDelayFilter.cpp
  Algorithms/mitkNavigationDataDisplacementFilter.cpp
  Algorithms/mitkNavigationDataEvaluationFilter.cpp
  Algorithms/mitkNavigationDataLandmarkTransformFilter.cpp
  Algorithms/mitkNavigationDataPassThroughFilter.cpp
  Algorithms/mitkNavigationDataReferenceTransformFilter.cpp
  Algorithms/mitkNavigationDataSmoothingFilter.cpp
  Algorithms/mitkNavigationDataToMessageFilter.cpp
  Algorithms/mitkNavigationDataToNavigationDataFilter.cpp
  Algorithms/mitkNavigationDataToPointSetFilter.cpp
  Algorithms/mitkNavigationDataTransformFilter.cpp
  Algorithms/mitkNavigationDataToIGTLMessageFilter.cpp
  Algorithms/mitkIGTLMessageToNavigationDataFilter.cpp
  Algorithms/mitkNeedleProjectionFilter.cpp
  Algorithms/mitkPivotCalibration.cpp

  Common/mitkIGTTimeStamp.cpp
  Common/mitkSerialCommunication.cpp

  DataManagement/mitkNavigationDataSource.cpp
  DataManagement/mitkNavigationTool.cpp
  DataManagement/mitkNavigationToolStorage.cpp
  DataManagement/mitkTrackingDeviceSourceConfigurator.cpp
  DataManagement/mitkTrackingDeviceSource.cpp
  DataManagement/mitkTrackingDeviceTypeCollection.cpp

  ExceptionHandling/mitkIGTException.cpp
  ExceptionHandling/mitkIGTHardwareException.cpp
  ExceptionHandling/mitkIGTIOException.cpp

  IO/mitkNavigationDataPlayer.cpp
  IO/mitkNavigationDataPlayerBase.cpp
  IO/mitkNavigationDataRecorder.cpp
  IO/mitkNavigationDataRecorderDeprecated.cpp
  IO/mitkNavigationDataSequentialPlayer.cpp
  IO/mitkNavigationToolReader.cpp
  IO/mitkNavigationToolStorageSerializer.cpp
  IO/mitkNavigationToolStorageDeserializer.cpp
  IO/mitkNavigationToolWriter.cpp
  IO/mitkNavigationDataReaderInterface.cpp

  Rendering/mitkCameraVisualization.cpp
  Rendering/mitkNavigationDataObjectVisualizationFilter.cpp
  Rendering/mitkNavigationDataSliceVisualization.cpp

  TrackingDevices/mitkClaronTool.cpp
  TrackingDevices/mitkClaronTrackingDevice.cpp
  TrackingDevices/mitkNDIPassiveTool.cpp
  TrackingDevices/mitkNDIProtocol.cpp
  TrackingDevices/mitkNDITrackingDevice.cpp
  TrackingDevices/mitkTrackingDevice.cpp
  TrackingDevices/mitkTrackingTool.cpp
  TrackingDevices/mitkTrackingVolumeGenerator.cpp
  TrackingDevices/mitkVirtualTrackingDevice.cpp
  TrackingDevices/mitkVirtualTrackingTool.cpp
  TrackingDevices/mitkOptitrackErrorMessages.cpp
  TrackingDevices/mitkOptitrackTrackingDevice.cpp
  TrackingDevices/mitkOptitrackTrackingTool.cpp
  TrackingDevices/mitkOpenIGTLinkTrackingDevice.cpp
  TrackingDevices/mitkOpenIGTLinkTrackingTool.cpp
  TrackingDevices/mitkNDIAuroraTypeInformation.cpp
  TrackingDevices/mitkNDIPolarisTypeInformation.cpp
  TrackingDevices/mitkNPOptitrackTrackingTypeInformation.cpp
  TrackingDevices/mitkVirtualTrackerTypeInformation.cpp
  TrackingDevices/mitkMicronTrackerTypeInformation.cpp
  TrackingDevices/mitkOpenIGTLinkTypeInformation.cpp
  TrackingDevices/mitkUnspecifiedTrackingTypeInformation.cpp
#  TrackingDevices/mitkPolhemusTrackingDevice.cpp
#  TrackingDevices/mitkPolhemusTool.cpp
#  TrackingDevices/mitkPolhemusTrackerTypeInformation.cpp
)

set(H_FILES
  DataManagement/mitkTrackingDeviceTypeInformation.h
  Common/mitkTrackingTypes.h
)

set(RESOURCE_FILES
  ClaronMicron.stl
  IntuitiveDaVinci.stl
  NDIAurora.stl
  NDIAurora_Dome.stl
  NDIAuroraCompactFG_Dome.stl
  NDIAuroraPlanarFG_Dome.stl
  NDIAuroraTabletopFG_Dome.stl
  NDIAuroraTabletopFG_Prototype_Dome.stl
  NDIPolarisOldModel.stl
  NDIPolarisSpectra.stl
  NDIPolarisSpectraExtendedPyramid.stl
  NDIPolarisVicra.stl
)

if(MITK_USE_MICRON_TRACKER)
  set(CPP_FILES ${CPP_FILES} TrackingDevices/mitkClaronInterface.cpp)
else()
  set(CPP_FILES ${CPP_FILES} TrackingDevices/mitkClaronInterfaceStub.cpp)
endif(MITK_USE_MICRON_TRACKER)

if(MITK_USE_MICROBIRD_TRACKER)
  set(CPP_FILES ${CPP_FILES} TrackingDevices/mitkMicroBirdTrackingDevice.cpp)
endif(MITK_USE_MICROBIRD_TRACKER)

if(MITK_USE_POLHEMUS_TRACKER)
  set(CPP_FILES ${CPP_FILES}
  TrackingDevices/mitkPolhemusInterface.cpp
  TrackingDevices/mitkPolhemusTrackingDevice.cpp
  TrackingDevices/mitkPolhemusTool.cpp
  TrackingDevices/mitkPolhemusTrackerTypeInformation.cpp
)
endif(MITK_USE_POLHEMUS_TRACKER)
