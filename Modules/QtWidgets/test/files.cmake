# test fails easily on MacOS, rarely on Windows, needs to be fixed before permanent activation (bug 15479)
set(MODULE_TESTS)

if(BUG_15479_FIXED)
  list(APPEND MODULE_TESTS QmitkThreadedLogTest.cpp)
endif()

set(MODULE_CUSTOM_TESTS
  QmitkDataStorageListModelTest.cpp
  QmitkDataStorageTreeModelTest.cpp
  QmitkAbstractNodeSelectionWidgetTest.cpp
  QmitkButtonOverlayWidgetTest.cpp
  QmitkIconThemeTest.cpp
  QmitkMxNExplicitNameTest.cpp
  QmitkMxNGeometryAuthorityTest.cpp
  QmitkMxNLayoutV2Test.cpp
  QmitkMxNLayoutV3Test.cpp
  QmitkMxNGridOpsTest.cpp
  QmitkMxNNavLinksTest.cpp
  QmitkMxNLayoutEditorWidgetTest.cpp
  QmitkMxNArrangeModeTest.cpp
  QmitkMxNCellOverlayTest.cpp
  QmitkMxNNavigatorTest.cpp
  QmitkMxNSyncGroupApiTest.cpp
  QmitkMxNSynchronizeScopeTest.cpp
  QmitkMxNSyncPeekTest.cpp
  QmitkMxNDataBasedLayoutTest.cpp
  QmitkRenderWindowProximityTest.cpp
  QmitkSynchronizedWidgetConnectorTest.cpp
)
