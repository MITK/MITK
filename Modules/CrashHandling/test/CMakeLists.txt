configure_file(mitkCrashHandlingTestConfig.h.in mitkCrashHandlingTestConfig.h)

include_directories(../src ${CMAKE_CURRENT_BINARY_DIR})
MITK_CREATE_MODULE_TESTS()

#[[ The helper must be created after MITK_CREATE_MODULE_TESTS: creating it
    overwrites MODULE_NAME, from which the test driver name is derived. ]]
mitk_create_executable(CrashDumpTestHelper
  DEPENDS MitkCrashHandling
  CPP_FILES MitkCrashDumpTestHelper.cpp
  NO_FEATURE_INFO
  NO_BATCH_FILE
  NO_INSTALL
)

if(TARGET ${TESTDRIVER} AND TARGET ${EXECUTABLE_TARGET})
  add_dependencies(${TESTDRIVER} ${EXECUTABLE_TARGET})
  if(TARGET MitkCopyCrashpadHandler)
    add_dependencies(${TESTDRIVER} MitkCopyCrashpadHandler)
  endif()
  set_property(TEST mitkCrashDumpCaptureTest PROPERTY TIMEOUT 120)
endif()
