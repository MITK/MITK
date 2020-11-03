# test fails easily on MacOS, rarely on Windows, needs to be fixed before permanent activation (bug 15479)
set(MODULE_TESTS)

if(BUG_15479_FIXED)
  list(APPEND MODULE_TESTS QmitkThreadedLogTest.cpp)
endif()

set(MODULE_CUSTOM_TESTS
  QmitkDataStorageListModelTest.cpp
  QmitkAbstractNodeSelectionWidgetTest.cpp
)
