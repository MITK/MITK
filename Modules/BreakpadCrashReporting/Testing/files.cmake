set(MODULE_TESTS
  if(CMAKE_SYSTEM MATCHES "Windows") # currently only for windows (see documentation of test mitkBreakpadCrashReportingDumpTest)
    mitkBreakpadCrashReportingDumpTest.cpp
  endif()
)
