SET(MODULE_TESTS
   mitkCameraVisualizationTest.cpp
   mitkClaronInterfaceTest.cpp
   mitkClaronToolTest.cpp
   mitkClaronTrackingDeviceTest.cpp
   mitkInternalTrackingToolTest.cpp
   mitkNavigationDataDisplacementFilterTest.cpp
   mitkNavigationDataLandmarkTransformFilterTest.cpp
   mitkNavigationDataObjectVisualizationFilterTest.cpp
   mitkNavigationDataTest.cpp
   mitkNavigationDataRecorderTest.cpp
   mitkNavigationDataReferenceTransformFilterTest.cpp
   mitkNavigationDataSequentialPlayerTest.cpp
   mitkNavigationDataToMessageFilterTest.cpp
   mitkNavigationDataToNavigationDataFilterTest.cpp
   mitkNavigationDataToPointSetFilterTest.cpp
   mitkNavigationDataTransformFilterTest.cpp
   mitkNDIPassiveToolTest.cpp
   mitkNDIProtocolTest.cpp
   mitkNDITrackingDeviceTest.cpp
   mitkTimeStampTest.cpp
   mitkTrackingVolumeGeneratorTest.cpp
   mitkTrackingDeviceTest.cpp
   mitkTrackingToolTest.cpp
# This test fails randomly, see bug #8033
   mitkVirtualTrackingDeviceTest.cpp
   mitkNavigationDataPlayerTest.cpp
   mitkTrackingDeviceSourceTest.cpp
   mitkTrackingDeviceSourceConfiguratorTest.cpp
   mitkNavigationDataEvaluationFilterTest.cpp
   mitkTrackingTypesTest.cpp

   # ------------------ Navigation Tool Management Tests -------------------
   mitkNavigationToolStorageTest.cpp
   mitkNavigationToolStorageSerializerAndDeserializerTest.cpp
   mitkNavigationToolTest.cpp
   mitkNavigationToolReaderAndWriterTest.cpp #deactivated, see bug #3461
   # -----------------------------------------------------------------------

   # ------------------ Deavtivated Tests ----------------------------------

   # -----------------------------------------------------------------------
)

SET(MODULE_CUSTOM_TESTS
  mitkNDIAuroraHardwareTest.cpp
  mitkNDIPolarisHardwareTest.cpp
  mitkClaronTrackingDeviceHardwareTest.cpp
)
