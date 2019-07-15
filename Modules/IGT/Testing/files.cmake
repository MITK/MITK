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


   ################# RUNNING TESTS #######################################################
   mitkCameraVisualizationTest.cpp
   mitkClaronInterfaceTest.cpp
   mitkClaronToolTest.cpp
   mitkClaronTrackingDeviceTest.cpp
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
   mitkNavigationDataToIGTLMessageFilterTest.cpp
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
   mitkOpenIGTLinkTrackingDeviceTest.cpp
   # ------------------ Navigation Tool Management Tests -------------------
   mitkNavigationToolStorageDeserializerTest.cpp # Activated experimentally on dart clients, see task T17303 for details.
   mitkNavigationToolStorageTest.cpp
   mitkNavigationToolTest.cpp
   # -----------------------------------------------------------------------
)

set(MODULE_CUSTOM_TESTS
  mitkNDIAuroraHardwareTest.cpp
  mitkNDIPolarisHardwareTest.cpp
  mitkClaronTrackingDeviceHardwareTest.cpp
  mitkNavigationToolReaderAndWriterTest.cpp #deactivated because of bug 18835
  mitkNavigationToolStorageSerializerAndDeserializerIntegrationTest.cpp # This test was disabled because of bug 17181.
  mitkNavigationToolStorageSerializerTest.cpp # This test was disabled because of bug 18671
)

if(MITK_USE_POLHEMUS_TRACKER)
set(MODULE_CUSTOM_TESTS
  ${MODULE_CUSTOM_TESTS}
  mitkPolhemusTrackingDeviceHardwareTest.cpp
  mitkPolhemusToolTest.cpp
)
endif(MITK_USE_POLHEMUS_TRACKER)
