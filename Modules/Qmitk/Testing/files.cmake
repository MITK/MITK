# test fails on MacOS, needs to be fixed before permanent activation (bug 15479)
if (NOT CMAKE_HOST_APPLE)

  set(MODULE_TESTS
    QmitkThreadedLogTest.cpp
  )

endif()