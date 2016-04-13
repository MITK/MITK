# test fails easily on MacOS, rarely on Windows, needs to be fixed before permanent activation (bug 15479)
if (BUG_15479_FIXED)

  set(MODULE_TESTS
    QmitkThreadedLogTest.cpp
  )

endif()


set(MODULE_TESTS ${MODULE_TESTS}
  QmitkDataStorageListModelTest.cpp
)
