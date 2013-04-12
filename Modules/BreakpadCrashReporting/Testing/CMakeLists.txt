if(BUILD_TESTING)
  MITK_CREATE_MODULE_TESTS()

  set_property(TEST mitkBreakpadCrashReportingDumpTest PROPERTY WILL_FAIL TRUE) # this one SHOULD crash
  set_property(TEST mitkBreakpadCrashReportingDumpCheckTest PROPERTY DEPENDS mitkBreakpadCrashReportingDumpTest) # this one checks if a dump was created

endif()
