set(MODULE_TESTS
   # IMPORTANT: If you plan to deactivate / comment out a test please write a bug number to the commented out line of code.
   #
   #     Example: #mitkMyTest #this test is commented out because of bug 12345
   #
   # It is important that the bug is open and that the test will be activated again before the bug is closed. This assures that
   # no test is forgotten after it was commented out. If there is no bug for your current problem, please add a new one and
   # mark it as critical.

   ################## ON THE FENCE TESTS #################################################
   # none

   ################## DISABLED TESTS #####################################################
   # mitkNavigationToolStorageDeserializerTest.cpp # This test was disabled because of bug 17303.
   # mitkNavigationToolStorageSerializerAndDeserializerIntegrationTest.cpp # This test was disabled because of bug 17181.

   ################# RUNNING TESTS #######################################################
   mitkCameraVisualizationTest.cpp
   mitkClaronInterfaceTest.cpp
   mitkClaronToolTest.cpp
   mitkClaronTrackingDeviceTest.cpp
   mitkInternalTrackingToolTest.cpp
   mitkNavigationDataDisplacementFilterTest.cpp
   mitkNavigationDataLandmarkTransformFilterTest.cpp
   mitkNavigationDataObjectVisualizationFilterTest.cpp
   mitkNavigationDataSetTest.cpp
   mitkNavigationDataTest.cpp
   mitkNavigationDataRecorderTest.cpp
   mitkNavigationDataReferenceTransformFilterTest.cpp
   mitkNavigationDataSequentialPlayerTest.cpp
   mitkNavigationDataSetReaderWriterXMLTest.cpp
   mitkNavigationDataSetReaderWriterCSVTest.cpp
   mitkNavigationDataSourceTest.cpp
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
   mitkVirtualTrackingDeviceTest.cpp
   # mitkNavigationDataPlayerTest.cpp # random fails see bug 16485.
   # We decided to won't fix because of complete restructuring via bug 15959.
   mitkTrackingDeviceSourceTest.cpp
   mitkTrackingDeviceSourceConfiguratorTest.cpp
   mitkNavigationDataEvaluationFilterTest.cpp
   mitkTrackingTypesTest.cpp
   # ------------------ Navigation Tool Management Tests -------------------
   mitkNavigationToolStorageTest.cpp
   mitkNavigationToolTest.cpp
   mitkNavigationToolReaderAndWriterTest.cpp
   mitkNavigationToolStorageSerializerTest.cpp
   # -----------------------------------------------------------------------
)

set(MODULE_CUSTOM_TESTS
  mitkNDIAuroraHardwareTest.cpp
  mitkNDIPolarisHardwareTest.cpp
  mitkClaronTrackingDeviceHardwareTest.cpp
)
