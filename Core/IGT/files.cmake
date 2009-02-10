SET(CPP_FILES
  IGTFilters/mitkNavigationDataLandmarkTransformFilter.cpp
  IGTFilters/mitkNavigationDataTransformFilter.cpp
  IGTFilters/mitkNavigationDataRecorder.cpp
  IGTFilters/mitkCameraVisualization.cpp
  IGTFilters/mitkNavigationData.cpp
  IGTFilters/mitkNavigationDataDisplacementFilter.cpp
  IGTFilters/mitkNavigationDataSource.cpp
  IGTFilters/mitkNavigationDataToNavigationDataFilter.cpp
  IGTFilters/mitkNavigationDataToPointSetFilter.cpp
  IGTFilters/mitkNavigationDataVisualizationFilter.cpp
  IGTFilters/mitkTrackingDeviceSource.cpp
  #IGTFilters/mitkTimeStamp.cpp ## remove until meaningful implementation is ready
  
  IGTTrackingDevices/mitkClaronTool.cpp
  IGTTrackingDevices/mitkClaronTrackingDevice.cpp
  IGTTrackingDevices/mitkTrackingDeviceConfigurator.cpp
  IGTTrackingDevices/mitkTrackingDevice.cpp
  IGTTrackingDevices/mitkTrackingTool.cpp
  IGTTrackingDevices/mitkTrackingVolume.cpp
)

IF(MITK_USE_MICRON_TRACKER)
  SET(CPP_FILES ${CPP_FILES} IGTTrackingDevices/mitkClaronInterface.cpp)
ELSE()
  SET(CPP_FILES ${CPP_FILES} IGTTrackingDevices/mitkClaronInterfaceStub.cpp)
ENDIF(MITK_USE_MICRON_TRACKER)

IF(MITK_USE_MICROBIRD_TRACKER)
  SET(CPP_FILES ${CPP_FILES} IGTTrackingDevices/mitkMicroBirdTrackingDevice.cpp 
                             IGTTrackingDevices/mitkMicroBirdTool.cpp )
ENDIF(MITK_USE_MICROBIRD_TRACKER)  
  
  